#Requires -Version 5.1
<#
.SYNOPSIS
    Validates the ACE-Step VST3 plugin bundle after a real CUDA Toolkit build.

.DESCRIPTION
    Checks that the expected DLLs are present in the bundle directory and that the
    plugin binary does not directly import CUDA runtime dependencies.  CUDA loading
    is expected to go through ggml-cuda.dll only.

    Run from a VS 2022 Developer PowerShell so that dumpbin.exe is on the PATH.

.PARAMETER BuildDir
    Root of the CMake build tree.  Defaults to the build-real directory next to the
    ACE-Step-Plugin repository root.

.PARAMETER Config
    CMake build configuration (e.g. RelWithDebInfo, Release).  Defaults to
    RelWithDebInfo.

.EXAMPLE
    .\validate-bundle.ps1

.EXAMPLE
    .\validate-bundle.ps1 -BuildDir C:\builds\ace-real -Config Release
#>
param(
    [string]$BuildDir = "$PSScriptRoot\..\build-real",
    [string]$Config   = "RelWithDebInfo"
)

$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

function Write-Pass {
    param([string]$Message)
    Write-Host "[PASS] $Message" -ForegroundColor Green
}

function Write-Fail {
    param([string]$Message)
    Write-Host "[FAIL] $Message" -ForegroundColor Red
}

$failures = [System.Collections.Generic.List[string]]::new()

function Add-Failure {
    param([string]$Message)
    Write-Fail $Message
    $failures.Add($Message)
}

# ---------------------------------------------------------------------------
# Resolve bundle directory
# ---------------------------------------------------------------------------

$bundleDir = Join-Path $BuildDir "AceStepPlugin_artefacts\$Config\VST3\ACE-Step.vst3\Contents\x86_64-win"

Write-Host "Build dir  : $BuildDir"
Write-Host "Config     : $Config"
Write-Host "Bundle dir : $bundleDir"
Write-Host ""

if (-not (Test-Path $BuildDir)) {
    Write-Fail "Build directory not found: $BuildDir"
    Write-Host ""
    Write-Host "Build the plugin first with a real CUDA Toolkit, then re-run this script."
    $repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..") -ErrorAction SilentlyContinue
    if ($repoRoot) {
        Write-Host "Expected build root: $($repoRoot.Path)\build-real"
    }
    exit 1
}

if (-not (Test-Path $bundleDir)) {
    Write-Fail "Bundle directory not found: $bundleDir"
    Write-Host ""
    Write-Host "Expected path: $bundleDir"
    Write-Host "Ensure the VST3 target was built with --config $Config and the build succeeded."
    exit 1
}

# ---------------------------------------------------------------------------
# Verify plugin binary
# ---------------------------------------------------------------------------

$pluginBinary = Join-Path $bundleDir "AceStepPlugin.vst3"

if (-not (Test-Path $pluginBinary)) {
    Add-Failure "Plugin binary not found: $pluginBinary"
} else {
    Write-Pass "Plugin binary exists: AceStepPlugin.vst3"
}

# ---------------------------------------------------------------------------
# Verify required DLLs
# ---------------------------------------------------------------------------

$requiredDlls = @(
    "ggml-base.dll",
    "ggml-cpu.dll",
    "ggml-cuda.dll",
    "ggml-vulkan.dll"
)

foreach ($dll in $requiredDlls) {
    $dllPath = Join-Path $bundleDir $dll
    if (-not (Test-Path $dllPath)) {
        Add-Failure "Required DLL missing: $dll"
    } else {
        Write-Pass "Required DLL present: $dll"
    }
}

# ---------------------------------------------------------------------------
# Run dumpbin to inspect direct dependencies of the plugin binary
# ---------------------------------------------------------------------------

if ($failures.Count -eq 0) {
    # Only run dumpbin when the plugin binary exists.
    $dumpbinCmd = Get-Command dumpbin.exe -ErrorAction SilentlyContinue
    if (-not $dumpbinCmd) {
        throw "dumpbin.exe not found on PATH.  Re-run this script from a VS 2022 Developer PowerShell " +
              "(search for 'Developer PowerShell for VS 2022' in the Start menu)."
    }
    $dumpbinExe = $dumpbinCmd.Source

    Write-Host ""
    Write-Host "Running: dumpbin /dependents `"$pluginBinary`""

    $dumpbinOutput = & $dumpbinExe /dependents "$pluginBinary" 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "dumpbin failed (exit $LASTEXITCODE).  Ensure you are running from a VS 2022 Developer PowerShell."
    }

    # Extract DLL names from dumpbin output.
    # dumpbin prints lines like "    kernel32.dll" under the "Image has the following dependencies:" header.
    $dependents = [System.Collections.Generic.List[string]]::new()
    $inSection  = $false

    foreach ($line in $dumpbinOutput) {
        if ($line -match "Image has the following dependencies") {
            $inSection = $true
            continue
        }
        if ($inSection) {
            # A blank line or a "Summary" header ends the section.
            if ($line -match "^\s*$" -or $line -match "^\s*Summary") {
                $inSection = $false
                continue
            }
            $trimmed = $line.Trim()
            if ($trimmed -ne "" -and $trimmed -match "\.dll$") {
                $dependents.Add($trimmed.ToLower())
            }
        }
    }

    Write-Host ""
    Write-Host "Direct dependents ($($dependents.Count)):"
    foreach ($dep in $dependents) {
        Write-Host "  $dep"
    }
    Write-Host ""

    # Patterns that must NOT appear as direct imports of the plugin DLL.
    # ggml-cuda.dll is the intended CUDA boundary; the plugin must not bypass it.
    $forbiddenPatterns = @(
        "^cudart64_",
        "^nvcuda\.dll$",
        "^cublas64_"
    )

    foreach ($dep in $dependents) {
        foreach ($pattern in $forbiddenPatterns) {
            if ($dep -match $pattern) {
                Add-Failure "Plugin DLL directly imports a CUDA dependency: $dep  " +
                            "(CUDA must load only through ggml-cuda.dll)"
            }
        }
    }
} else {
    Write-Host ""
    Write-Host "Skipping dumpbin check because earlier checks failed." -ForegroundColor Yellow
}

# ---------------------------------------------------------------------------
# Final result
# ---------------------------------------------------------------------------

Write-Host ""

if ($failures.Count -eq 0) {
    Write-Pass "Bundle structure is correct and no direct CUDA imports found."
    exit 0
} else {
    Write-Host "--- $($failures.Count) failure(s) ---" -ForegroundColor Red
    foreach ($f in $failures) {
        Write-Host "  * $f" -ForegroundColor Red
    }
    exit 1
}
