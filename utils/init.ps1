<#
    Configuration Initialization Script
    Generates .vscode/sftp.json and dev/dev_config.json
#>

$ErrorActionPreference = "Stop"
$ScriptRoot = $PSScriptRoot
# ProjectRoot is one level up from utils
$ProjectRoot = Split-Path -Parent $ScriptRoot

# Define paths
$DevDir = Join-Path $ProjectRoot "dev"
$SftpExamplePath = Join-Path $DevDir "sftp.json.example"
$DevConfigPath = Join-Path $DevDir "dev_config.json"
$VsCodeDir = Join-Path $ProjectRoot ".vscode"
$SftpConfigPath = Join-Path $VsCodeDir "sftp.json"

# 1. Define Constants
$IntranetIP = "10.201.98.97"
$VpnIP = "10.0.1.2"
$Port = 6022
$ProjectName = "final_os"

Write-Host "=== Project Configuration Init ===" -ForegroundColor Cyan

# 2. Collect User Information
$User = Read-Host "Enter your Username"
if ([string]::IsNullOrWhiteSpace($User)) {
    Write-Error "Username is required."
    exit 1
}

$Pass = Read-Host "Enter your Password"
if ([string]::IsNullOrWhiteSpace($Pass)) {
    Write-Error "Password is required."
    exit 1
}

# 3. Select Network Mode
Write-Host "`nSelect Network Mode:"
Write-Host "  [N] Intranet (Default): $IntranetIP"
Write-Host "  [Y] VPN               : $VpnIP"
$NetChoice = Read-Host "Use VPN? [y/N]"

if ($NetChoice -match "^[yY]") {
    $HostIP = $VpnIP
    Write-Host "-> Selected: VPN ($HostIP)" -ForegroundColor Green
} else {
    $HostIP = $IntranetIP
    Write-Host "-> Selected: Intranet ($HostIP)" -ForegroundColor Green
}

# 4. Generate .vscode/sftp.json
if (-not (Test-Path $SftpExamplePath)) {
    Write-Error "File not found: $SftpExamplePath"
    exit 1
}

# Ensure .vscode directory exists
if (-not (Test-Path $VsCodeDir)) {
    New-Item -ItemType Directory -Path $VsCodeDir | Out-Null
}

try {
    $JsonContent = Get-Content $SftpExamplePath -Raw | ConvertFrom-Json
    
    $RemotePath = "/home/$User/projects/$ProjectName"

    $JsonContent.host = $HostIP
    $JsonContent.port = $Port
    $JsonContent.username = $User
    $JsonContent.password = $Pass
    $JsonContent.remotePath = $RemotePath

    # Update ignore list to exclude the dev config directory to prevent leaking credentials
    if ($JsonContent.ignore -notcontains "dev/dev_config.json") {
        $JsonContent.ignore += "dev/dev_config.json"
    }

    $JsonContent | ConvertTo-Json -Depth 5 | Set-Content $SftpConfigPath
    Write-Host "`n[OK] Generated: .vscode/sftp.json" -ForegroundColor Green
} catch {
    Write-Error "Failed to generate sftp.json: $_"
}

# 5. Generate dev/dev_config.json
try {
    $DevConfig = @{
        HostIP     = $HostIP
        Port       = $Port
        Username   = $User
        Password   = $Pass
        RemotePath = $RemotePath
    }
    $DevConfig | ConvertTo-Json | Set-Content $DevConfigPath
    Write-Host "[OK] Generated: dev/dev_config.json" -ForegroundColor Green
} catch {
    Write-Error "Failed to generate dev_config.json: $_"
}

# 6. Final Instructions
Write-Host "`n=== Initialization Complete ===" -ForegroundColor Cyan
Write-Host "Next Steps:"
Write-Host "1. Run 'SFTP: Upload Project' (Ctrl+Shift+P)."
Write-Host "2. Run '.\dev.ps1 connect' to verify SSH connection."
