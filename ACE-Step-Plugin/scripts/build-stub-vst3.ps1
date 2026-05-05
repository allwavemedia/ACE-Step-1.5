#Requires -Version 5.1
<#
.SYNOPSIS
    Build the ACE-Step stub VST3 plugin without CUDA/ACE-Step C++ dependencies.

.DESCRIPTION
    Configures and builds the AceStepPlugin_VST3 CMake target using stub mode
    (ACESTEP_ENABLE_ACESTEP_CPP=OFF, ACESTEP_BUILD_TESTS=OFF).  The resulting
    bundle can be loaded in AudioPluginHost or Reaper for host-load validation
    as described in ACE-Step-Plugin\docs\validate-host-load.md.

.PARAMETER BuildDir
    Directory where CMake will write its build files.
    Defaults to ACE-Step-Plugin\build-vst3-stub.

.PARAMETER Config
    CMake build configuration (Debug, Release, RelWithDebInfo, MinSizeRel).
    Defaults to RelWithDebInfo.

.EXAMPLE
    .\build-stub-vst3.ps1
    .\build-stub-vst3.ps1 -BuildDir C:\tmp\my-build -Config Debug
#>
param(
    [string]$BuildDir = "$PSScriptRoot\..\build-vst3-stub",
    [string]$Config   = "RelWithDebInfo"
)

$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Resolve paths
# ---------------------------------------------------------------------------

$PluginRoot = (Resolve-Path "$PSScriptRoot\..").Path
$BuildDir   = [System.IO.Path]::GetFullPath($BuildDir)

Write-Host "Plugin root : $PluginRoot"
Write-Host "Build dir   : $BuildDir"
Write-Host "Config      : $Config"
Write-Host ""

# ---------------------------------------------------------------------------
# CMake configure
# ---------------------------------------------------------------------------

Write-Host "=== CMake configure ==="
cmake `
    -S $PluginRoot `
    -B $BuildDir `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DACESTEP_ENABLE_ACESTEP_CPP=OFF `
    -DACESTEP_BUILD_TESTS=OFF

if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed with exit code $LASTEXITCODE."
}

# ---------------------------------------------------------------------------
# CMake build
# ---------------------------------------------------------------------------

Write-Host ""
Write-Host "=== CMake build (target: AceStepPlugin_VST3, config: $Config) ==="
cmake --build $BuildDir --config $Config --target AceStepPlugin_VST3 --parallel

if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed with exit code $LASTEXITCODE."
}

# ---------------------------------------------------------------------------
# Verify bundle exists
# ---------------------------------------------------------------------------

$BundlePath = Join-Path $BuildDir "AceStepPlugin_artefacts\$Config\VST3\ACE-Step.vst3"

Write-Host ""
Write-Host "=== Verifying bundle ==="
if (-not (Test-Path $BundlePath)) {
    throw "Expected VST3 bundle not found at: $BundlePath"
}

Write-Host "Bundle found: $BundlePath"

# ---------------------------------------------------------------------------
# Success
# ---------------------------------------------------------------------------

Write-Host ""
Write-Host "SUCCESS: Stub VST3 built successfully."
Write-Host ""
Write-Host "Next steps:"
Write-Host "  Load the bundle in AudioPluginHost or Reaper to validate host"
Write-Host "  compatibility.  See ACE-Step-Plugin\docs\validate-host-load.md"
Write-Host "  for the full validation checklist."
Write-Host ""
Write-Host "  Bundle path: $BundlePath"
