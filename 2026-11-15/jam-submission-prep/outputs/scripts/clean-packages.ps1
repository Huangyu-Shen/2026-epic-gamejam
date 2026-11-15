#requires -Version 5.1
<#
.SYNOPSIS
  Lists the old package folders and, with -Apply, deletes the ones you do not
  need. Dry run by default: nothing is removed unless you pass -Apply.

.DESCRIPTION
  Each package under Game_01 5.8\Saved\Packages is roughly 0.7-1.1 GB, and the
  folder fills up fast during a jam. The newest folder and any name in -Keep are
  always kept.

.PARAMETER Keep
  Names (not paths) that must survive, for example submission-2026-11-15-ab12cd3.

.PARAMETER KeepNewest
  How many of the newest folders to keep. Default 1.

.PARAMETER Apply
  Really delete. Without it the script only prints what it would delete.

.EXAMPLE
  pwsh -File .\clean-packages.ps1
  pwsh -File .\clean-packages.ps1 -Keep "submission-2026-11-15-ab12cd3" -Apply
#>
[CmdletBinding()]
param(
    [string[]]$Keep = @(),
    [int]$KeepNewest = 1,
    [switch]$Apply
)

$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\_common.ps1"

$PackagesDir = Get-PackagesDir
if (-not (Test-Path -LiteralPath $PackagesDir)) {
    Write-Fail "No packages folder at $PackagesDir"
    exit 2
}

$all = @(Get-ChildItem -LiteralPath $PackagesDir -Directory | Sort-Object LastWriteTime -Descending)
if ($all.Count -eq 0) {
    Write-Note 'No package folders; nothing to do.'
    exit 0
}

$protected = @($all | Select-Object -First $KeepNewest)
foreach ($k in $Keep) {
    $match = $all | Where-Object { $_.Name -eq $k }
    if ($match) { $protected += $match } else { Write-Warn "Keep name not found: $k" }
}
$protected = @($protected | Sort-Object FullName -Unique)

$total = 0.0
Write-Step "Packages in $PackagesDir"
foreach ($p in $all) {
    $size = Get-DirSize -Path $p.FullName
    $total += $size
    $mark = if ($protected -contains $p) { 'KEEP' } else { 'delete' }
    $color = if ($mark -eq 'KEEP') { 'Green' } else { 'DarkGray' }
    Write-Host ("  {0,-6} {1,-28} {2,10}  {3}" -f $mark, $p.Name, (Format-Size $size), $p.LastWriteTime.ToString('MM-dd HH:mm')) -ForegroundColor $color
}
Write-Note "total: $(Format-Size $total)"

$doomed = @($all | Where-Object { $protected -notcontains $_ })
$reclaim = 0.0
foreach ($d in $doomed) { $reclaim += Get-DirSize -Path $d.FullName }

Write-Step "Plan"
Write-Note "delete $($doomed.Count) folder(s), reclaim $(Format-Size $reclaim)"

if (-not $Apply) {
    Write-Note 'Dry run. Re-run with -Apply to delete.'
    exit 0
}

foreach ($d in $doomed) {
    try {
        Remove-Item -LiteralPath $d.FullName -Recurse -Force
        Write-Ok "deleted $($d.Name)"
    } catch {
        Write-Fail "could not delete $($d.Name): $($_.Exception.Message)"
    }
}
exit 0
