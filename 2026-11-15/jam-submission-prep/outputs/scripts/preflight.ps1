#requires -Version 5.1
<#
.SYNOPSIS
  Read-only pre-submission check. Changes nothing.

.DESCRIPTION
  Checks the things that silently broke before: missing cook entries, the Zen
  AutoLaunch line, editor-only plugins, an unclean tree, package and zip sizes,
  video resolution/duration, and leftover placeholders in the submission text.

  Exit code 0 = ready, 1 = at least one FAIL, 2 = could not run.
  WARN (yellow) means "look at it yourself", not "broken".

.PARAMETER ZipPath
  Submission zip to check. Default: newest *.zip under outputs\dist.

.PARAMETER VideoPath
  Final video to check. Default: newest *.mp4 under outputs\dist.

.EXAMPLE
  pwsh -File .\preflight.ps1
#>
[CmdletBinding()]
param(
    [string]$ZipPath,
    [string]$VideoPath
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\_common.ps1"

$RepoRoot   = Get-RepoRoot
$ProjectDir = Get-ProjectDir
$DistDir    = Join-Path (Get-OutputsDir) 'dist'
$results    = New-Object System.Collections.ArrayList

function Add-Result {
    param([string]$Level, [string]$Name, [string]$Detail = '')
    [void]$results.Add([pscustomobject]@{ Level = $Level; Name = $Name; Detail = $Detail })
    switch ($Level) {
        'OK'   { Write-Ok   ("{0}{1}" -f $Name, $(if ($Detail) { " - $Detail" } else { '' })) }
        'WARN' { Write-Warn ("{0}{1}" -f $Name, $(if ($Detail) { " - $Detail" } else { '' })) }
        'FAIL' { Write-Fail ("{0}{1}" -f $Name, $(if ($Detail) { " - $Detail" } else { '' })) }
        default { Write-Note ("{0}{1}" -f $Name, $(if ($Detail) { " - $Detail" } else { '' })) }
    }
}

Write-Step "Repository"
$sha      = Get-HeadSha -RepoRoot $RepoRoot
$branch   = Get-HeadBranch -RepoRoot $RepoRoot
$modified = @(& git -C $RepoRoot status --porcelain --untracked-files=no 2>$null | Where-Object { $_ })
$untracked = @(& git -C $RepoRoot status --porcelain 2>$null | Where-Object { $_ -like '??*' })
Write-Note "head: $sha on $branch"

if ($modified.Count -eq 0) {
    Add-Result 'OK' 'Working tree (tracked files) is clean'
} else {
    Add-Result 'WARN' "Working tree has $($modified.Count) modified tracked file(s)" 'story 013 wants a named commit; record the sha in smoke-2026-11-15.md'
}
if ($untracked.Count -gt 0) {
    $untrackedContent = @($untracked | Where-Object { $_ -match 'Content/' })
    Add-Result 'WARN' "$($untracked.Count) untracked item(s), $($untrackedContent.Count) under Content/" 'the Fab packs are untracked on purpose; the package must be built on this machine'
} else {
    Add-Result 'OK' 'No untracked files'
}

$lfsCount = @(& git -C $RepoRoot lfs ls-files 2>$null).Count
if ($lfsCount -gt 0) { Add-Result 'OK' "Git LFS tracks $lfsCount file(s)" } else { Add-Result 'WARN' 'Git LFS reports no files' 'check .gitattributes and git lfs install' }

Write-Step "Toolchain"
$engineRoot = 'D:/Game/EPIC/UE_5.8'
$uat = Join-Path $engineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
if (Test-Path -LiteralPath $uat) { Add-Result 'OK' "Unreal BuildCookRun found" $engineRoot } else { Add-Result 'FAIL' 'RunUAT.bat not found' $uat }
if (Test-EditorRunning) { Add-Result 'FAIL' 'Unreal Editor is running' 'close it before packaging' } else { Add-Result 'OK' 'Editor is not running' }

$free = (Get-PSDrive -Name ((Get-Item -LiteralPath $RepoRoot).PSDrive.Name)).Free
if ($free -gt 8GB) { Add-Result 'OK' "Free space on the repo drive: $(Format-Size $free)" } else { Add-Result 'WARN' "Low free space: $(Format-Size $free)" 'a package plus a staging copy needs about 3 GB' }

Write-Step "Packaging configuration"
$engineIni = Join-Path $ProjectDir 'Config\DefaultEngine.ini'
$gameIni   = Join-Path $ProjectDir 'Config\DefaultGame.ini'

if (Test-Path -LiteralPath $engineIni) {
    $zen = Select-String -LiteralPath $engineIni -Pattern 'AutoLaunch\s*=\s*False' -SimpleMatch:$false
    if ($zen) {
        Add-Result 'FAIL' 'DefaultEngine.ini still has AutoLaunch=False' 'this broke cook and stage twice (day 2 dev log)'
    } else {
        Add-Result 'OK' 'No Zen AutoLaunch=False'
    }
} else {
    Add-Result 'FAIL' 'DefaultEngine.ini missing' $engineIni
}

if (Test-Path -LiteralPath $gameIni) {
    $gameText = Get-Content -LiteralPath $gameIni -Raw
    foreach ($map in '/Game/Maps/L_Camp', '/Game/Maps/L_Greybox') {
        if ($gameText -match [regex]::Escape($map)) { Add-Result 'OK' "MapsToCook has $map" }
        else { Add-Result 'FAIL' "MapsToCook is missing $map" 'START RUN fails in the package without it' }
    }
    foreach ($dir in '/Game/Data', '/Game/Free_Sounds_Pack/cue', '/Game/PathOfAdventure/Loop/CUE') {
        if ($gameText -match [regex]::Escape($dir)) { Add-Result 'OK' "DirectoriesToAlwaysCook has $dir" }
        else { Add-Result 'FAIL' "DirectoriesToAlwaysCook is missing $dir" 'soft references are skipped silently by the cooker' }
    }
} else {
    Add-Result 'FAIL' 'DefaultGame.ini missing' $gameIni
}

$uproject = Get-ProjectPath
$upText = Get-Content -LiteralPath $uproject -Raw
foreach ($plugin in 'ModelContextProtocol', 'AllToolsets', 'Terminal') {
    $block = [regex]::Match($upText, '"Name"\s*:\s*"' + $plugin + '"[\s\S]{0,200}?\}')
    if ($block.Success -and $block.Value -match 'TargetAllowList') {
        Add-Result 'OK' "$plugin is restricted to some targets"
    } else {
        Add-Result 'WARN' "$plugin has no TargetAllowList" 'editor-only plugins are not limited; packaging change, needs the author approval'
    }
}

Write-Step "Build output"
$pkg = Get-LatestPackage
if ($pkg) {
    $exe    = Join-Path $pkg.FullName 'Windows\Game_01\Binaries\Win64\Game_01.exe'
    $pdb    = Join-Path $pkg.FullName 'Windows\Game_01\Binaries\Win64\Game_01.pdb'
    $shipping = Test-Path -LiteralPath (Join-Path $pkg.FullName 'Windows\Game_01\Binaries\Win64\Game_01-Win64-Shipping.exe')
    $total  = Get-DirSize -Path $pkg.FullName
    $pdbLen = if (Test-Path -LiteralPath $pdb) { (Get-Item -LiteralPath $pdb).Length } else { 0 }
    Add-Result 'OK' "Newest package: $($pkg.Name)" ("total $(Format-Size $total), without .pdb $(Format-Size ($total - $pdbLen))")
    $configLabel = if ($shipping) { 'Shipping' } else { 'Development (the jam page asks for a development build, so this is acceptable)' }
    Add-Result 'OK' "Package configuration: $configLabel"
} else {
    Add-Result 'WARN' 'No finished package under Saved\Packages' 'run package-shipping.ps1 first'
}

Write-Step "Submission files"
if (-not $ZipPath -and (Test-Path -LiteralPath $DistDir)) {
    $z = Get-ChildItem -LiteralPath $DistDir -Filter '*.zip' -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($z) { $ZipPath = $z.FullName }
}
if ($ZipPath -and (Test-Path -LiteralPath $ZipPath)) {
    $zb = (Get-Item -LiteralPath $ZipPath).Length
    if ($zb -le 1000000000) { Add-Result 'OK' "Zip is under 1 GB: $(Format-Size $zb)" }
    else { Add-Result 'FAIL' "Zip is over 1 GB: $(Format-Size $zb)" 'remove content or rebuild without debug symbols' }
    if ((Split-Path -Leaf $ZipPath) -notmatch '^[^_]+_[^_]+\.zip$') {
        Add-Result 'WARN' "Zip name is not TeamName_GameName.zip" (Split-Path -Leaf $ZipPath)
    } else {
        Add-Result 'OK' 'Zip name follows TeamName_GameName.zip'
    }
    Add-Result 'OK' "Zip sha256" (Get-FileHash -LiteralPath $ZipPath -Algorithm SHA256).Hash
} else {
    Add-Result 'WARN' 'No submission zip found' 'run make-submission-zip.ps1'
}

if (-not $VideoPath -and (Test-Path -LiteralPath $DistDir)) {
    $v = Get-ChildItem -LiteralPath $DistDir -Filter '*.mp4' -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($v) { $VideoPath = $v.FullName }
}
if ($VideoPath -and (Test-Path -LiteralPath $VideoPath)) {
    $ffprobe = Get-Command ffprobe -ErrorAction SilentlyContinue
    if ($ffprobe) {
        $duration = & ffprobe -v error -show_entries format=duration -of csv=p=0 "$VideoPath" 2>$null
        $wh = & ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=p=0:s=x "$VideoPath" 2>$null
        $dur = 0.0
        [void][double]::TryParse(($duration | Select-Object -First 1), [ref]$dur)
        $whText = ($wh | Select-Object -First 1)
        Add-Result 'OK' "Video: $([System.IO.Path]::GetFileName($VideoPath))" ("$whText, {0:N1} s" -f $dur)
        if ($dur -lt 30 -or $dur -gt 60) { Add-Result 'FAIL' 'Video length is outside 30-60 s' ("{0:N1} s" -f $dur) }
        if ($whText -match '^(\d+)x(\d+)$' -and [int]$Matches[1] -lt 1920) { Add-Result 'WARN' 'Video width is below 1920' 'self-imposed target; the jam page only requires 30-60 seconds' }
    } else {
        Add-Result 'WARN' 'ffprobe not installed, video not probed' 'check length 30-60 s and at least 1920x1080 by hand, or install Gyan.FFmpeg'
    }
} else {
    Add-Result 'WARN' 'No video found under outputs\dist' 'record it with 02-video-plan.md / 03-video-script.md'
}

$subText = Join-Path $RepoRoot 'docs\submission\submission-text.md'
if (Test-Path -LiteralPath $subText) {
    $st = Get-Content -LiteralPath $subText -Raw
    if ($st -match 'TO ADD') { Add-Result 'WARN' 'submission-text.md still has "TO ADD" placeholders' 'fill the video and build links' }
    else { Add-Result 'OK' 'submission-text.md has no "TO ADD" left' }
    if ($st -match '\*\*CHECK\*\*') { Add-Result 'WARN' 'submission-text.md still has CHECK items' 'e.g. open the dev log logged out' }
} else {
    Add-Result 'FAIL' 'submission text missing' $subText
}

Write-Step "Summary"
$fails = @($results | Where-Object { $_.Level -eq 'FAIL' }).Count
$warns = @($results | Where-Object { $_.Level -eq 'WARN' }).Count
$oks   = @($results | Where-Object { $_.Level -eq 'OK' }).Count
Write-Host ("  {0} OK   {1} WARN   {2} FAIL" -f $oks, $warns, $fails) -ForegroundColor $(if ($fails) { 'Red' } elseif ($warns) { 'Yellow' } else { 'Green' })

if ($fails) {
    Write-Host '  Not ready. Fix the FAIL lines above.' -ForegroundColor Red
    exit 1
}
Write-Host '  No blocking problems found. Work through 04-pre-submit-checklist.md next.' -ForegroundColor Green
exit 0
