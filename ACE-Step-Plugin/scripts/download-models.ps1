<#
.SYNOPSIS
    Downloads GGUF model files required by ACE-Step-Plugin into the local models directory.

.DESCRIPTION
    Fetches the four ACE-Step 1.5 GGUF models from HuggingFace and places them in
    %LOCALAPPDATA%\AceStepPlugin\models (or a custom path via -ModelsDir).
    Source repository: Serveurperso/ACE-Step-1.5-GGUF

    NOTE: If the HuggingFace repository is private, authentication will be required
    (e.g. set HF_TOKEN env var and pass -Headers @{Authorization="Bearer $env:HF_TOKEN"}
    to Invoke-WebRequest). No credentials are hardcoded in this script.

.PARAMETER ModelsDir
    Directory in which to store the downloaded models.
    Defaults to "$env:LOCALAPPDATA\AceStepPlugin\models".

.PARAMETER SkipDownload
    When set, missing models are reported but no network requests are made.
    Use this flag for dry-run validation or offline environments.

.EXAMPLE
    .\download-models.ps1

.EXAMPLE
    .\download-models.ps1 -ModelsDir "D:\MyModels"

.EXAMPLE
    .\download-models.ps1 -ModelsDir "$env:TEMP\test-models-dryrun" -SkipDownload
#>

param(
    [string]$ModelsDir = "$env:LOCALAPPDATA\AceStepPlugin\models",
    [switch]$SkipDownload
)

$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Model manifest
# ---------------------------------------------------------------------------
# Model manifest — aligned with Resources/model_manifest.json
# Source: Serveurperso/ACE-Step-1.5-GGUF
$Models = @(
    @{
        Filename     = "acestep-5Hz-lm-4B-Q5_K_M.gguf"
        Url          = "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/acestep-5Hz-lm-4B-Q5_K_M.gguf"
        ExpectedSize = 3025965984
    },
    @{
        Filename     = "acestep-v15-turbo-Q5_K_M.gguf"
        Url          = "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/acestep-v15-turbo-Q5_K_M.gguf"
        ExpectedSize = 1700140224
    },
    @{
        Filename     = "Qwen3-Embedding-0.6B-Q8_0.gguf"
        Url          = "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/Qwen3-Embedding-0.6B-Q8_0.gguf"
        ExpectedSize = 784144960
    },
    @{
        Filename     = "vae-BF16.gguf"
        Url          = "https://huggingface.co/Serveurperso/ACE-Step-1.5-GGUF/resolve/main/vae-BF16.gguf"
        ExpectedSize = 337420928
    }
)

# ---------------------------------------------------------------------------
# Ensure models directory exists
# ---------------------------------------------------------------------------
if (-not (Test-Path -LiteralPath $ModelsDir)) {
    Write-Host "Creating models directory: $ModelsDir"
    New-Item -ItemType Directory -Path $ModelsDir -Force | Out-Null
}
else {
    Write-Host "Models directory: $ModelsDir"
}

# ---------------------------------------------------------------------------
# Process each model
# ---------------------------------------------------------------------------
$PresentCount = 0

foreach ($Model in $Models) {
    $DestPath = Join-Path $ModelsDir $Model.Filename
    $PartPath = "$DestPath.part"

    if (Test-Path -LiteralPath $DestPath) {
        $ExistingFile = Get-Item -LiteralPath $DestPath
        $SizeMB = [math]::Round($ExistingFile.Length / 1MB, 1)
        if ($Model.ExpectedSize -gt 0 -and $ExistingFile.Length -ne $Model.ExpectedSize) {
            Write-Host "MISMATCH $($Model.Filename) (expected $($Model.ExpectedSize) bytes, found $($ExistingFile.Length))"
            if ($SkipDownload) {
                continue
            }
            Write-Host "  Re-downloading $($Model.Filename)"
            Remove-Item -LiteralPath $DestPath -Force
        }
        else {
            Write-Host "SKIP   $($Model.Filename) (already present, $SizeMB MB)"
            $PresentCount++
            continue
        }
    }

    if ($SkipDownload) {
        Write-Host "MISSING $($Model.Filename) (skipping download as requested)"
        continue
    }

    Write-Host "DOWNLOAD $($Model.Filename) ..."
    Write-Host "  URL: $($Model.Url)"

    # Remove any stale partial file before starting
    if (Test-Path -LiteralPath $PartPath) {
        Remove-Item -LiteralPath $PartPath -Force
    }

    try {
        Invoke-WebRequest `
            -Uri $Model.Url `
            -OutFile $PartPath `
            -Headers @{ "User-Agent" = "ACE-Step-Plugin/0.1" } `
            -UseBasicParsing

        Move-Item -LiteralPath $PartPath -Destination $DestPath
        $SizeMB = [math]::Round((Get-Item -LiteralPath $DestPath).Length / 1MB, 1)
        Write-Host "  OK - saved $SizeMB MB to $DestPath"
        $PresentCount++
    }
    catch {
        Write-Warning "Failed to download $($Model.Filename): $_"
        # Clean up partial file on failure; do not leave a corrupt file behind
        if (Test-Path -LiteralPath $PartPath) {
            Remove-Item -LiteralPath $PartPath -Force -ErrorAction SilentlyContinue
        }
        # Continue to check/download remaining models
    }
}

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
$RequiredCount = $Models.Count
Write-Host ""
Write-Host "Models directory : $ModelsDir"
Write-Host "Required         : $RequiredCount"
Write-Host "Present          : $PresentCount"

if ($PresentCount -lt $RequiredCount) {
    Write-Host "INCOMPLETE - $($RequiredCount - $PresentCount) model(s) missing."
    Write-Host "If HuggingFace requires authentication, set HF_TOKEN and add an Authorization header locally."
    exit 1
}

Write-Host "All models present."
exit 0
