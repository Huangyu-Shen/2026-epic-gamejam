#requires -Version 5.1
<#
.SYNOPSIS
  Collects the final files (video, zip, texts, raw footage hashes) into one
  dated bundle with a manifest.

.DESCRIPTION
  Copies - it never moves or deletes anything. Produces:

    outputs\dist\submission-<sha>\
      build\   the submission zip
      video\   the final mp4 and its .srt
      text\    submission text and the prep documents
      raw\     *.mkv / *.mp4 footage from outputs\raw (hashed, not copied if huge)
      manifest.md, SHA256SUMS.txt

  If ffprobe is installed it records resolution, duration and frame rate for
  every video, and warns when the final video breaks the 30-60 s / 1920x1080
  rules.

.PARAMETER VideoPath
  Final cut. Default: newest *.mp4 under outputs\dist.

.PARAMETER ZipPath
  Submission zip. Default: newest *.zip under outputs\dist.

.PARAMETER RawDir
  Folder with OBS recordings. Default: outputs\raw (skipped when missing).

.PARAMETER IncludeRaw
  Copy the raw footage into the bundle. Off by default: it is usually many GB,
  the manifest lists it either way.

.EXAMPLE
  pwsh -File .\archive-materials.ps1 -VideoPath "D:\edit\DirectorsCut-final.mp4"
#>
[CmdletBinding()]
param(
    [string]$VideoPath,
    [string]$ZipPath,
    [string]$RawDir,
    [switch]$IncludeRaw
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\_common.ps1"

$RepoRoot = Get-RepoRoot
$Outputs  = Get-OutputsDir
$DistDir  = Join-Path $Outputs 'dist'
$Sha      = Get-HeadSha -RepoRoot $RepoRoot
if (-not $RawDir) { $RawDir = Join-Path $Outputs 'raw' }

New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
$Bundle = Join-Path $DistDir "submission-$Sha"
foreach ($sub in 'build', 'video', 'text', 'raw') {
    New-Item -ItemType Directory -Force -Path (Join-Path $Bundle $sub) | Out-Null
}

Write-Step "Collecting into $Bundle"

# --- video ---------------------------------------------------------------
if (-not $VideoPath -and (Test-Path -LiteralPath $DistDir)) {
    $v = Get-ChildItem -LiteralPath $DistDir -Filter '*.mp4' -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($v) { $VideoPath = $v.FullName }
}
$videoTarget = $null
if ($VideoPath -and (Test-Path -LiteralPath $VideoPath)) {
    $videoTarget = Join-Path $Bundle ("video\" + (Split-Path -Leaf $VideoPath))
    Copy-Item -LiteralPath $VideoPath -Destination $videoTarget -Force
    Write-Ok "video: $videoTarget"
    $srt = [System.IO.Path]::ChangeExtension($VideoPath, '.srt')
    if (Test-Path -LiteralPath $srt) {
        Copy-Item -LiteralPath $srt -Destination (Join-Path $Bundle 'video') -Force
        Write-Ok "subtitles: $(Split-Path -Leaf $srt)"
    } else {
        Write-Warn 'no .srt next to the video (03-video-script.srt can be used as a base)'
    }
} else {
    Write-Warn 'no final video found; the bundle will be missing one'
}

# --- zip -----------------------------------------------------------------
if (-not $ZipPath -and (Test-Path -LiteralPath $DistDir)) {
    $z = Get-ChildItem -LiteralPath $DistDir -Filter '*.zip' -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($z) { $ZipPath = $z.FullName }
}
if ($ZipPath -and (Test-Path -LiteralPath $ZipPath)) {
    Copy-Item -LiteralPath $ZipPath -Destination (Join-Path $Bundle 'build') -Force
    Write-Ok "build: $(Join-Path $Bundle 'build')"
} else {
    Write-Warn 'no submission zip found; run make-submission-zip.ps1 first'
}

# --- texts ---------------------------------------------------------------
$texts = @(
    (Join-Path $RepoRoot 'docs\submission\submission-text.md')
    (Join-Path $Outputs '01-package-and-submit-plan.md')
    (Join-Path $Outputs '02-video-plan.md')
    (Join-Path $Outputs '03-video-script.md')
    (Join-Path $Outputs '03-video-script.srt')
    (Join-Path $Outputs '04-pre-submit-checklist.md')
)
foreach ($t in $texts) {
    if (Test-Path -LiteralPath $t) {
        Copy-Item -LiteralPath $t -Destination (Join-Path $Bundle 'text') -Force
    } else {
        Write-Note "skipped (not there yet): $(Split-Path -Leaf $t)"
    }
}
Write-Ok "text: $(Join-Path $Bundle 'text')"

# --- raw footage ---------------------------------------------------------
$rawFiles = @()
if (Test-Path -LiteralPath $RawDir) {
    $rawFiles = @(Get-ChildItem -LiteralPath $RawDir -File -Include '*.mkv', '*.mp4', '*.mov' -Recurse)
    if ($IncludeRaw) {
        foreach ($f in $rawFiles) {
            Copy-Item -LiteralPath $f.FullName -Destination (Join-Path $Bundle 'raw') -Force
        }
        Write-Ok "raw footage copied: $($rawFiles.Count) file(s)"
    } else {
        Write-Note "raw footage listed only (pass -IncludeRaw to copy): $($rawFiles.Count) file(s)"
    }
} else {
    Write-Note "no raw folder at $RawDir (fine if you recorded elsewhere)"
}

# --- probe videos --------------------------------------------------------
$ffprobe = Get-Command ffprobe -ErrorAction SilentlyContinue
function Get-VideoFacts {
    param([string]$Path)
    if (-not $ffprobe) { return $null }
    $w = & ffprobe -v error -select_streams v:0 -show_entries stream=width -of csv=p=0 "$Path" 2>$null
    $h = & ffprobe -v error -select_streams v:0 -show_entries stream=height -of csv=p=0 "$Path" 2>$null
    $r = & ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate -of csv=p=0 "$Path" 2>$null
    $d = & ffprobe -v error -show_entries format=duration -of csv=p=0 "$Path" 2>$null
    return [pscustomobject]@{
        Width  = ($w | Select-Object -First 1)
        Height = ($h | Select-Object -First 1)
        Fps    = ($r | Select-Object -First 1)
        Seconds = ($d | Select-Object -First 1)
    }
}

$videoFacts = $null
if ($videoTarget) {
    $videoFacts = Get-VideoFacts -Path $videoTarget
    if ($videoFacts) {
        $sec = 0.0
        [void][double]::TryParse($videoFacts.Seconds, [ref]$sec)
        Write-Note ("video facts: {0}x{1} @ {2} fps, {3:N1} s" -f $videoFacts.Width, $videoFacts.Height, $videoFacts.Fps, $sec)
        $bad = $false
        if ($sec -lt 30 -or $sec -gt 60) { Write-Fail ("length {0:N1} s is outside 30-60 s" -f $sec); $bad = $true }
        if ([int]$videoFacts.Width -lt 1920) { Write-Fail ("width {0} is below 1920" -f $videoFacts.Width); $bad = $true }
        if (-not $bad) { Write-Ok 'video fits the 30-60 s and 1920x1080+ rules' }
    } else {
        Write-Warn 'ffprobe not installed: check length and resolution by hand (winget install Gyan.FFmpeg)'
    }
}

# --- hashes and manifest -------------------------------------------------
Write-Step "Hashes"
$entries = New-Object System.Collections.ArrayList
$hashLines = New-Object System.Collections.ArrayList

$distFiles = @(Get-ChildItem -LiteralPath $Bundle -Recurse -File |
               Where-Object { $_.Name -notin @('manifest.md', 'SHA256SUMS.txt') })
foreach ($f in $distFiles) {
    $h = (Get-FileHash -LiteralPath $f.FullName -Algorithm SHA256).Hash
    $rel = $f.FullName.Substring($Bundle.Length + 1)
    [void]$entries.Add([pscustomobject]@{ Path = $rel; Bytes = $f.Length; Sha256 = $h })
    [void]$hashLines.Add(("{0}  {1}" -f $h.ToLower(), $rel))
}
foreach ($f in $rawFiles) {
    $h = (Get-FileHash -LiteralPath $f.FullName -Algorithm SHA256).Hash
    [void]$entries.Add([pscustomobject]@{ Path = "raw(source)\$($f.Name)"; Bytes = $f.Length; Sha256 = $h })
}

$manifest = Join-Path $Bundle 'manifest.md'
$lines = New-Object System.Collections.ArrayList
[void]$lines.Add('# Submission bundle')
[void]$lines.Add('')
[void]$lines.Add("- commit: ``$Sha`` ($(Get-HeadBranch -RepoRoot $RepoRoot))")
[void]$lines.Add("- created: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
[void]$lines.Add('- deadline: 2026-11-16 01:59 UK time = 2026-11-16 09:59 Beijing time')
[void]$lines.Add('')
if ($videoFacts) {
    [void]$lines.Add('## Video')
    [void]$lines.Add('')
    [void]$lines.Add(("- {0}x{1}, {2} fps, {3} s" -f $videoFacts.Width, $videoFacts.Height, $videoFacts.Fps, $videoFacts.Seconds))
    [void]$lines.Add('')
}
[void]$lines.Add('## Files')
[void]$lines.Add('')
[void]$lines.Add('| File | Size | SHA-256 |')
[void]$lines.Add('|---|---|---|')
foreach ($e in $entries) {
    [void]$lines.Add(("| {0} | {1} | {2} |" -f $e.Path, (Format-Size $e.Bytes), $e.Sha256.Substring(0, 16) + '...'))
}
[void]$lines.Add('')
[void]$lines.Add('Full hashes are in `SHA256SUMS.txt`.')
$lines | Set-Content -LiteralPath $manifest -Encoding UTF8
$hashLines | Set-Content -LiteralPath (Join-Path $Bundle 'SHA256SUMS.txt') -Encoding UTF8

Write-Ok "manifest: $manifest"
Write-Note "bundle size: $(Format-Size (Get-DirSize -Path $Bundle))"
exit 0
