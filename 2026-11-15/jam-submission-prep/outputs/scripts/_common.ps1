#requires -Version 5.1
# Shared helpers for the jam-submission-prep scripts. Dot-source it:
#   . "$PSScriptRoot\_common.ps1"
# Everything here is read-only.

$Script:RepoRoot = $null

function Get-RepoRoot {
    # Walk up from this script until the folder that holds the Unreal project.
    param([string]$Start = $PSScriptRoot)
    $dir = Get-Item -LiteralPath $Start
    while ($dir) {
        $candidate = Join-Path $dir.FullName 'Game_01 5.8\Game_01.uproject'
        if (Test-Path -LiteralPath $candidate) { return $dir.FullName }
        $dir = $dir.Parent
    }
    throw "Could not find 'Game_01 5.8/Game_01.uproject' above $Start."
}

function Get-ProjectPath { Join-Path (Get-RepoRoot) 'Game_01 5.8\Game_01.uproject' }
function Get-ProjectDir  { Join-Path (Get-RepoRoot) 'Game_01 5.8' }
function Get-PackagesDir { Join-Path (Get-ProjectDir) 'Saved\Packages' }
function Get-OutputsDir  { Join-Path (Get-RepoRoot) '2026-11-15\jam-submission-prep\outputs' }

function Get-HeadSha {
    param([string]$RepoRoot = (Get-RepoRoot))
    $sha = & git -C $RepoRoot rev-parse --short HEAD 2>$null
    if ($LASTEXITCODE -ne 0) { return 'nogit' }
    return ($sha | Select-Object -First 1).Trim()
}

function Get-HeadBranch {
    param([string]$RepoRoot = (Get-RepoRoot))
    $b = & git -C $RepoRoot rev-parse --abbrev-ref HEAD 2>$null
    if ($LASTEXITCODE -ne 0) { return 'nogit' }
    return ($b | Select-Object -First 1).Trim()
}

function Format-Size {
    param([double]$Bytes)
    if ($Bytes -ge 1GB) { return '{0:N2} GB' -f ($Bytes / 1GB) }
    if ($Bytes -ge 1MB) { return '{0:N1} MB' -f ($Bytes / 1MB) }
    if ($Bytes -ge 1KB) { return '{0:N0} KB' -f ($Bytes / 1KB) }
    return "$Bytes B"
}

function Get-DirSize {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) { return 0 }
    $sum = (Get-ChildItem -LiteralPath $Path -Recurse -File -ErrorAction SilentlyContinue |
            Measure-Object -Property Length -Sum).Sum
    if ($null -eq $sum) { return 0 }
    return [double]$sum
}

function Write-Step { param([string]$Text) Write-Host ''; Write-Host "== $Text" -ForegroundColor Cyan }
function Write-Ok   { param([string]$Text) Write-Host "  [ OK ] $Text" -ForegroundColor Green }
function Write-Note { param([string]$Text) Write-Host "  [INFO] $Text" -ForegroundColor Gray }
function Write-Warn { param([string]$Text) Write-Host "  [WARN] $Text" -ForegroundColor Yellow }
function Write-Fail { param([string]$Text) Write-Host "  [FAIL] $Text" -ForegroundColor Red }

function Get-LatestPackage {
    # Newest folder under Saved/Packages that looks like a finished package.
    param([string]$PackagesDir = (Get-PackagesDir))
    if (-not (Test-Path -LiteralPath $PackagesDir)) { return $null }
    Get-ChildItem -LiteralPath $PackagesDir -Directory |
        Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'Windows\Game_01.exe') } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
}

function Test-EditorRunning {
    return [bool](Get-Process -Name 'UnrealEditor', 'UnrealEditor-Cmd', 'UnrealEditorWin64' -ErrorAction SilentlyContinue)
}
