#requires -Version 5.1
<#
.SYNOPSIS
  Turns a built package into the submission zip.

.DESCRIPTION
  Takes <package>\Windows\ and zips its CONTENTS at the zip root, so the file a
  player double-clicks (Game_01.exe) sits next to Engine\ and Game_01\.
  Excludes *.pdb, Saved\ and *.log. Fails if the zip is over the 1 GB jam limit.

.PARAMETER PackageDir
  Folder under Game_01 5.8\Saved\Packages\. Default: the newest finished package.

.PARAMETER OutputZip
  Default: <outputs>\dist\<TeamName>_DirectorsCut.zip

.PARAMETER TeamName
  Your team name, or your own name when entering solo. The jam requires the zip
  to be named TeamName_GameName. Default: KnightOnNight (the team's display name
  is "Knight On Night"; spaces are dropped so the file name has no spaces).

.PARAMETER IncludePdb
  Keep the debug symbols (about 358 MB). Do not use for the submission.

.PARAMETER Force
  Overwrite an existing zip.

.EXAMPLE
  pwsh -File .\make-submission-zip.ps1
  Produces KnightOnNight_DirectorsCut.zip

.EXAMPLE
  pwsh -File .\make-submission-zip.ps1 -TeamName "Another Team"

.EXAMPLE
  pwsh -File .\make-submission-zip.ps1 -PackageDir "...\Saved\Packages\submission-2026-11-15-ab12cd3"
#>
[CmdletBinding()]
param(
    [string]$PackageDir,
    [string]$OutputZip,
    [string]$TeamName = 'KnightOnNight',
    [switch]$IncludePdb,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\_common.ps1"

$RepoRoot = Get-RepoRoot
$Sha      = Get-HeadSha -RepoRoot $RepoRoot
$DistDir  = Join-Path (Get-OutputsDir) 'dist'
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

if (-not $PackageDir) {
    $latest = Get-LatestPackage
    if (-not $latest) {
        Write-Fail "No finished package found under $(Get-PackagesDir)"
        Write-Note "Build one first: package-shipping.ps1"
        exit 2
    }
    $PackageDir = $latest.FullName
}
if (-not (Test-Path -LiteralPath $PackageDir)) {
    Write-Fail "Package folder not found: $PackageDir"
    exit 2
}

$WindowsDir = Join-Path $PackageDir 'Windows'
$Launcher   = Join-Path $WindowsDir 'Game_01.exe'
if (-not (Test-Path -LiteralPath $Launcher)) {
    Write-Fail "$Launcher is missing; this folder is not a finished package."
    exit 2
}

$safeTeam = ($TeamName.Trim() -replace '[\\/:*?"<>|]', '' -replace '\s+', '')
if (-not $safeTeam) { $safeTeam = 'TeamName' }
if ($safeTeam -ne $TeamName) {
    Write-Note "TeamName '$TeamName' -> '$safeTeam' (spaces and illegal characters removed for the file name)."
}

if (-not $OutputZip) {
    $OutputZip = Join-Path $DistDir "${safeTeam}_DirectorsCut.zip"
}
if ($safeTeam -eq 'TeamName') {
    Write-Warn "zip name uses the placeholder 'TeamName'; the jam requires TeamName_GameName, so rename it (or rerun with -TeamName)."
}
if ((Test-Path -LiteralPath $OutputZip) -and -not $Force) {
    Write-Fail "Zip already exists: $OutputZip"
    Write-Note "Pass -Force to overwrite, or -OutputZip with another name."
    exit 2
}

$pkgName = Split-Path -Leaf $PackageDir
Write-Step "Zip $pkgName"
Write-Note "source : $WindowsDir"
Write-Note "target : $OutputZip"
Write-Note "head   : $Sha"

$staging = Join-Path $env:TEMP ("dc-zip-" + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $staging | Out-Null

try {
    Write-Step "Staging (excluding debug symbols and runtime folders)"
    $rcArgs = @($WindowsDir, $staging, '/E', '/XD', 'Saved', '/NFL', '/NDL', '/NJH', '/NJS', '/NP')
    if (-not $IncludePdb) { $rcArgs += @('/XF', '*.pdb', '*.log') }
    robocopy @rcArgs | Out-Null
    $rc = $LASTEXITCODE
    if ($rc -ge 8) {
        Write-Fail "robocopy failed with exit code $rc"
        exit 3
    }

    if (-not (Test-Path -LiteralPath (Join-Path $staging 'Game_01.exe'))) {
        Write-Fail "Staging lost Game_01.exe; aborting."
        exit 3
    }

    Write-Step "Compressing (this takes a while for ~0.5-0.7 GB)"
    if (Test-Path -LiteralPath $OutputZip) { Remove-Item -LiteralPath $OutputZip -Force }
    Add-Type -AssemblyName System.IO.Compression.FileSystem -ErrorAction SilentlyContinue
    [System.IO.Compression.ZipFile]::CreateFromDirectory(
        $staging, $OutputZip, [System.IO.Compression.CompressionLevel]::Optimal, $false)
}
finally {
    if (Test-Path -LiteralPath $staging) { Remove-Item -LiteralPath $staging -Recurse -Force -ErrorAction SilentlyContinue }
}

$zipItem  = Get-Item -LiteralPath $OutputZip
$zipBytes = [double]$zipItem.Length
$limit    = 1000000000   # 1 GB, decimal, as the jam states it
$hash     = (Get-FileHash -LiteralPath $OutputZip -Algorithm SHA256).Hash

Write-Step "Result"
Write-Ok "zip: $OutputZip"
Write-Note "size      : $(Format-Size $zipBytes)  ($($zipItem.Length) bytes)"
Write-Note "headroom  : $(Format-Size ($limit - $zipBytes)) against the 1 GB limit"
Write-Note "sha256    : $hash"

$manifest = Join-Path $DistDir "zip-$Sha.md"
@(
    "# Submission zip"
    ''
    "- built from package: ``$pkgName``"
    "- commit: ``$Sha`` ($(Get-HeadBranch -RepoRoot $RepoRoot))"
    "- file: ``$(Split-Path -Leaf $OutputZip)``"
    "- size: $(Format-Size $zipBytes) ($($zipItem.Length) bytes)"
    "- sha256: ``$hash``"
    "- debug symbols included: $([bool]$IncludePdb)"
    "- created: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
    ''
    "Zip root holds the contents of ``Windows\`` (Game_01.exe is at the root)."
) | Set-Content -LiteralPath $manifest -Encoding UTF8
Write-Note "manifest  : $manifest"

if ($zipBytes -gt $limit) {
    Write-Fail "Zip is over 1 GB. Do not upload it: remove content or rebuild without debug symbols."
    exit 4
}

Write-Ok "Under the 1 GB limit."
Write-Note "Next: unpack it into a NEW folder and start the game from there."
exit 0
