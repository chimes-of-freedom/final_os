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
Write-Host ">>> [1/3] Downloading artifacts from $HostIP..." -ForegroundColor Cyan
Write-Host "(Please enter your password if prompted)" -ForegroundColor Yellow

# Construct remote paths
$RemoteFloppy = "${RemotePath}/a.img"
$RemoteHdd = "${RemotePath}/100m.img.zip"

# Download files to ProjectRoot
# Note: Using scp separately to ensure clarity, or could be combined.
# We download to current directory (ProjectRoot) context.
Push-Location $ProjectRoot

try {
    # Download Floppy
    scp -P $Port "$User@${HostIP}:$RemoteFloppy" .
    
    # Download Compressed HDD
    scp -P $Port "$User@${HostIP}:$RemoteHdd" .
} catch {
    Write-Error "SCP Download failed. Please check your connection or if 'dev image' was run on server."
    Pop-Location
    exit 1
}

# 3. Decompress HDD Image
Write-Host ">>> [2/3] Decompressing 100m.img.zip..." -ForegroundColor Cyan

if (Test-Path "100m.img.zip") {
    try {
	# Use PowerShell native command to unzip, -Force to overwrite old files
        Expand-Archive -Path "100m.img.zip" -DestinationPath . -Force
    } catch {
        Write-Error "Decompression failed: $_"
        Pop-Location
        exit 1
    }
} else {
    Write-Error "Error: 100m.img.zip not found after download."
    Pop-Location
    exit 1
}

# 4. Run Bochs
Write-Host ">>> [3/3] Starting Bochs Debugger..." -ForegroundColor Cyan

if (Test-Path "bochsrc.win") {
    # Check if bochs is in PATH, otherwise this will error out
    try {
        # -q: skip start menu, -f: config file
        bochs -q -f bochsrc.win -debugger
    } catch {
        Write-Error "Failed to start Bochs. Is it installed and in your PATH?"
    }
} else {
    Write-Error "Error: bochsrc.win not found in project root."
}

Pop-Location
Write-Host ">>> Test session ended." -ForegroundColor Green
