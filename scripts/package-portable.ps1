<#
.SYNOPSIS
    Assembles the Star Client portable Windows archive.

.DESCRIPTION
    Produces a portable .zip: no installer, no registry writes, nothing
    dropped outside the extracted folder. Layout inside the archive:

        StarClient.exe
        StarClient-console.exe
        StarClient_updater.exe      (when built)
        <Qt runtime dlls>           via windeployqt
        qt.conf                     so Qt finds the bundled plugins
        plugins/                    Qt platform, imageformats, styles
        licenses/                   GPL-3.0 + attribution
        README.txt                  first-run notes

    Portable means the launcher must keep its data next to the executable
    when a `portable.txt` marker is present - the same convention the
    upstream project uses.

.PARAMETER BuildDir
    CMake build directory containing the compiled binaries.

.PARAMETER QtDir
    Qt installation prefix (the directory containing bin/, plugins/, lib/).

.PARAMETER Version
    Version string used in the archive name and the README.

.PARAMETER OutputDir
    Where the finished .zip is written. Defaults to dist/.

.EXAMPLE
    ./scripts/package-portable.ps1 -BuildDir build -QtDir C:\Qt\6.8.1\msvc2022_64 -Version 21.1.0
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$BuildDir,
    [Parameter(Mandatory = $true)][string]$QtDir,
    [string]$Version = "dev",
    [string]$OutputDir = "dist",
    [string]$Flavor = "windows-x64"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot

function Write-Step($msg) { Write-Host "  $msg" -ForegroundColor Cyan }
function Fail($msg) { Write-Host "  ! $msg" -ForegroundColor Red; exit 1 }

Write-Host "`nStar Client portable packaging - $Version ($Flavor)`n" -ForegroundColor Magenta

# --------------------------------------------------------------------------- #
# Locate the built binaries
# --------------------------------------------------------------------------- #
$stage = Join-Path $OutputDir "StarClient-$Version-$Flavor"
if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory -Path $stage -Force | Out-Null

$exeNames = @("StarClient.exe", "StarClient-console.exe", "StarClient_updater.exe")
$found = @()
foreach ($name in $exeNames) {
    $hit = Get-ChildItem -Path $BuildDir -Recurse -Filter $name -ErrorAction SilentlyContinue |
           Select-Object -First 1
    if ($hit) {
        Copy-Item $hit.FullName -Destination $stage -Force
        $found += $name
        Write-Step "binary: $name"
    }
}

if ($found -notcontains "StarClient.exe") {
    Fail "StarClient.exe was not found under '$BuildDir'. Did the build succeed?"
}

# --------------------------------------------------------------------------- #
# Qt runtime
# --------------------------------------------------------------------------- #
$windeploy = Join-Path $QtDir "bin/windeployqt.exe"
if (-not (Test-Path $windeploy)) {
    Fail "windeployqt.exe not found at '$windeploy'. Pass -QtDir pointing at the Qt prefix."
}

Write-Step "running windeployqt"
& $windeploy --release --no-translations --no-system-d3d-compiler --no-opengl-sw `
             --skip-plugin-types "sqldrivers" (Join-Path $stage "StarClient.exe") | Out-Null
if ($LASTEXITCODE -ne 0) { Fail "windeployqt failed with exit code $LASTEXITCODE" }

# windeployqt scatters plugins beside the exe; keep them tidy but loadable
$pluginDir = Join-Path $stage "plugins"
New-Item -ItemType Directory -Path $pluginDir -Force | Out-Null
foreach ($sub in @("platforms", "imageformats", "styles", "iconengines", "networkinformation", "tls")) {
    $src = Join-Path $stage $sub
    if (Test-Path $src) { Move-Item $src (Join-Path $pluginDir $sub) -Force }
}
@"
[Paths]
Prefix = .
Plugins = plugins
"@ | Set-Content -Path (Join-Path $stage "qt.conf") -Encoding ASCII
Write-Step "qt.conf written"

# --------------------------------------------------------------------------- #
# Portable marker + licences + first-run notes
# --------------------------------------------------------------------------- #
# The launcher reads this to decide between an installed and a portable data
# directory, so a portable archive never touches %APPDATA%.
"Star Client portable" | Set-Content -Path (Join-Path $stage "portable.txt") -Encoding ASCII

$licDir = Join-Path $stage "licenses"
New-Item -ItemType Directory -Path $licDir -Force | Out-Null
foreach ($f in @("LICENSE", "COPYING.md", "CREDITS.md", "README.md")) {
    $p = Join-Path $root $f
    if (Test-Path $p) { Copy-Item $p -Destination $licDir -Force }
}
$lgpl = Join-Path $QtDir "LICENSES"
if (Test-Path $lgpl) { Copy-Item $lgpl (Join-Path $licDir "qt") -Recurse -Force }
Write-Step "licences staged"

@"
Star Client $Version - portable Windows build
=============================================

Run StarClient.exe. Nothing is installed and nothing is written outside
this folder: the portable.txt marker next to the executable makes Star
Client keep instances, settings and downloads in .\data.

Requirements
------------
  * 64-bit Windows 10 or newer
  * Java 17 or newer for Minecraft 1.18+ (Java 21 recommended)
    Star Client can download a runtime for you on first launch.
  * OpenGL 3.2 capable graphics drivers

First run
---------
  1. StarClient.exe
  2. Pick a theme and a memory allocation in the Quick Start wizard.
  3. Sign in with a Microsoft account, or add an offline profile.

Notes
-----
  * OLED Black is the signature theme; Daylight and Midnight are also included.
  * Star Menu presets can be switched off per instance if a modpack conflicts.
  * Garbage collector presets (G1GC / ZGC / Shenandoah / ...) live in
    Settings -> Java & performance.

Licence
-------
Star Client is free software under the GNU General Public License v3.0.
See licenses\ for the full text and licenses\CREDITS.md for attribution.
Minecraft is a trademark of Mojang Synergies AB.
"@ | Set-Content -Path (Join-Path $stage "README.txt") -Encoding UTF8

# --------------------------------------------------------------------------- #
# Zip it
# --------------------------------------------------------------------------- #
New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
$zipPath = Join-Path $OutputDir "StarClient-$Version-$Flavor-portable.zip"
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }

Write-Step "compressing"
Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $zipPath -CompressionLevel Optimal

$size = [math]::Round((Get-Item $zipPath).Length / 1MB, 1)
Write-Host "`n  done: $zipPath ($size MB)`n" -ForegroundColor Green
Write-Output $zipPath
