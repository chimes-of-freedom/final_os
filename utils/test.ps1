<#
    Test Workflow Script
    1. Downloads compiled artifacts (a.img, 100m.img.zip) from server using SCP.
    2. Decompresses the hard disk image.
    3. Launches Bochs with debugger.
#>

$ErrorActionPreference = "Stop"
$ScriptRoot = $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptRoot
$ConfigPath = Join-Path $ProjectRoot "dev/dev_config.json"

# 1. Check Configuration
if (-not (Test-Path $ConfigPath)) {
    Write-Error "Configuration file not found. Please run '.\dev.ps1 init' first."
    exit 1
}

$Config = Get-Content $ConfigPath | ConvertFrom-Json
$HostIP = $Config.HostIP
$Port = $Config.Port
$User = $Config.Username
$RemotePath = $Config.RemotePath

# 2. Pull Artifacts (SCP)
Write-Host ">>> [1/4] Downloading artifacts from $HostIP..." -ForegroundColor Cyan
Write-Host "(Please enter your password if prompted)" -ForegroundColor Yellow

# Download files to ProjectRoot
Push-Location $ProjectRoot

try {
    # Download artifacts.zip
    & scp -P $Port "$User@${HostIP}:${RemotePath}/artifacts.zip" .
    if ($LASTEXITCODE -ne 0) {
        throw "SCP failed for artifacts.zip (exit $LASTEXITCODE)"
    }
} catch {
    Write-Error "SCP Download failed. Please check your connection or if 'dev image' was run on server."
    Pop-Location
    exit 1
}

# 3. Decompress artifacts.zip
Write-Host ">>> [2/4] Decompressing artifacts.zip..." -ForegroundColor Cyan

if (Test-Path "artifacts.zip") {
    try {
        # Use PowerShell native command to unzip, -Force to overwrite old files
        Expand-Archive -Path "artifacts.zip" -DestinationPath . -Force
    } catch {
        Write-Error "Decompression failed: $_"
        Pop-Location
        exit 1
    }
} else {
    Write-Error "Error: artifacts.zip not found after download."
    Pop-Location
    exit 1
}

# 4. Run Bochs
Write-Host ">>> [3/4] Starting Bochs Debugger..." -ForegroundColor Cyan

if (Test-Path "bochsrc.win") {
    # Check if bochs is in PATH, otherwise this will error out
    try {
        # -q: skip start menu, -f: config file
        bochs -q -f bochsrc.win
    } catch {
        Write-Error "Failed to start Bochs. Is it installed and in your PATH?"
    }
} else {
    Write-Error "Error: bochsrc.win not found in project root."
}

Pop-Location
Write-Host ">>> Test session ended." -ForegroundColor Green

# 5. Cleanup
Write-Host ">>> [4/4] Cleaning up downloaded artifacts..." -ForegroundColor Cyan
Remove-Item -Path "$ProjectRoot\artifacts.zip" -ErrorAction SilentlyContinue
Remove-Item -Path "$ProjectRoot\a.img" -ErrorAction SilentlyContinue
Remove-Item -Path "$ProjectRoot\80m.img" -ErrorAction SilentlyContinue
Remove-Item -Path "$ProjectRoot\kernel.elf" -ErrorAction SilentlyContinue
Write-Host ">>> Cleanup completed." -ForegroundColor Green
