#requires -Version 5.1
<#
.SYNOPSIS
  Builds a Windows package of the jam project with Unreal's BuildCookRun.

.DESCRIPTION
  Defaults to a Development package. The official jam page asks for a
  "development build", and Development is also the only configuration this
  project has verified end to end. -Config Shipping is available but optional:
  it has never been built here and strips the probes.

  The archive goes to  Game_01 5.8\Saved\Packages\<Name>\
  Output:               <archive>\Windows\Game_01.exe  (+ Game_01\Binaries\...)

  Run it from a normal PowerShell window, not from inside an agent sandbox:
  Unreal Build Tool writes to %LOCALAPPDATA% and a sandboxed process is denied.

.PARAMETER Config
  Development (default, the jam's "development build") or Shipping (optional).

.PARAMETER Name
  Archive folder name. Default: submission-<date>-<sha>.

.PARAMETER EngineRoot
  Engine install. Default: D:/Game/EPIC/UE_5.8 (also in project.yaml).

.PARAMETER NoDebugInfo
  Passes -nodebuginfo so no .pdb is staged (~358 MB saved). Not tried in this
  project yet; if UAT rejects the flag, rerun without it.

.PARAMETER SkipBuild
  Passes no -build: reuse existing binaries, only cook/stage/pak/archive.

.PARAMETER Force
  Continue even if the editor is running or the archive folder already exists.

.EXAMPLE
  pwsh -File .\package-shipping.ps1 -Config Development -Name verify-2026-11-15

.EXAMPLE
  pwsh -File .\package-shipping.ps1
#>
[CmdletBinding()]
param(
    [ValidateSet('Development', 'Shipping')][string]$Config = 'Development',
    [string]$Name,
    [string]$EngineRoot = 'D:/Game/EPIC/UE_5.8',
    [switch]$NoDebugInfo,
    [switch]$SkipBuild,
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\_common.ps1"

$RepoRoot    = Get-RepoRoot
$ProjectPath = Get-ProjectPath
$Uat         = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$Sha         = Get-HeadSha -RepoRoot $RepoRoot
$Date        = Get-Date -Format 'yyyy-MM-dd'

if (-not $Name) {
    $Name = "submission-$Date-$Sha"
}

Write-Step "Package $Name ($Config)"
Write-Note "repo     : $RepoRoot"
Write-Note "project  : $ProjectPath"
Write-Note "head     : $Sha on $(Get-HeadBranch -RepoRoot $RepoRoot)"
Write-Note "engine   : $EngineRoot"

if (-not (Test-Path -LiteralPath $Uat)) {
    Write-Fail "RunUAT.bat not found at $Uat"
    Write-Note "Pass -EngineRoot <engine install> if the engine lives elsewhere."
    exit 2
}

if (Test-EditorRunning) {
    if (-not $Force) {
        Write-Fail "Unreal Editor (or Live Coding) is running; close it first, or pass -Force."
        exit 2
    }
    Write-Warn "Editor is running; continuing because -Force was given. The build may fail with 'file in use'."
}

$ArchiveDir = Join-Path (Get-PackagesDir) $Name
if (Test-Path -LiteralPath $ArchiveDir) {
    if (-not $Force) {
        Write-Fail "Archive folder already exists: $ArchiveDir"
        Write-Note "Pick another -Name, or pass -Force to reuse (old contents will be overwritten)."
        exit 2
    }
    Write-Warn "Reusing existing archive folder: $ArchiveDir"
}

# The documented command shape is in project.yaml and docs/wiki/build-and-run.md section 8.
$uatArgs = @(
    "BuildCookRun"
    "-project=$ProjectPath"
    '-noP4'
    '-platform=Win64'
    "-clientconfig=$Config"
    '-cook'
    '-stage'
    '-pak'
    '-archive'
    "-archivedirectory=$ArchiveDir"
    '-utf8output'
    '-unattended'
)
if (-not $SkipBuild) { $uatArgs += '-build' }
if ($NoDebugInfo)    { $uatArgs += '-nodebuginfo' }

$LogDir = Join-Path (Get-OutputsDir) 'dist\logs'
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$LogPath = Join-Path $LogDir "package-$Name.log"

Write-Step "Running BuildCookRun"
Write-Note ('"{0}" {1}' -f $Uat, ($uatArgs -join ' '))
Write-Note "log: $LogPath"
$started = Get-Date

& $Uat @uatArgs 2>&1 | Tee-Object -FilePath $LogPath
$code = $LASTEXITCODE

Write-Step "Result"
if ($code -ne 0) {
    Write-Fail "BuildCookRun failed with exit code $code after $([int]((Get-Date) - $started).TotalMinutes) min."
    Write-Note "Last 40 lines of the log:"
    Get-Content -LiteralPath $LogPath -Tail 40 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
    Write-Note "Common causes are listed in 01-package-and-submit-plan.md section 6 (Zen AutoLaunch, editor open, UBT sandbox denial)."
    exit $code
}

$Exe    = Join-Path $ArchiveDir 'Windows\Game_01.exe'
$Pdb    = Join-Path $ArchiveDir 'Windows\Game_01\Binaries\Win64\Game_01.pdb'
if (-not (Test-Path -LiteralPath $Exe)) {
    Write-Fail "Build reported success but $Exe is missing."
    exit 3
}

$total = Get-DirSize -Path $ArchiveDir
$pdbSize = if (Test-Path -LiteralPath $Pdb) { (Get-Item -LiteralPath $Pdb).Length } else { 0 }

Write-Ok "Package built: $ArchiveDir"
Write-Note "archive total     : $(Format-Size $total)"
Write-Note "debug symbol .pdb : $(Format-Size $pdbSize)"
Write-Note "without .pdb      : $(Format-Size ($total - $pdbSize))"
Write-Note "launcher          : Windows\Game_01.exe"
Write-Note ''
Write-Note "Next: check the package in a clean folder, then run make-submission-zip.ps1."
exit 0
