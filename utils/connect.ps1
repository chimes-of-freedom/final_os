<#
    SSH Connection Script
    Reads config and connects to the server, jumping to the project directory.
#>

$ErrorActionPreference = "Stop"
$ScriptRoot = $PSScriptRoot
$ProjectRoot = Split-Path -Parent $ScriptRoot
$ConfigPath = Join-Path $ProjectRoot "dev/dev_config.json"

# 1. Check Configuration
if (-not (Test-Path $ConfigPath)) {
    Write-Error "Configuration file not found: $ConfigPath"
    Write-Error "Please run '.\dev.ps1 init' first."
    exit 1
}

$Config = Get-Content $ConfigPath | ConvertFrom-Json

# 2. Extract Credentials
$HostIP = $Config.HostIP
$Port = $Config.Port
$User = $Config.Username
$RemotePath = $Config.RemotePath

# 3. Construct SSH Command
# -t: Force pseudo-tty allocation (required for interactive shell)
# Command: cd to directory AND launch a new bash shell
$RemoteCommand = "cd $RemotePath && echo 'Welcome to OS Project!' && exec bash"

Write-Host "Connecting to $User@${HostIP}:${Port} ..." -ForegroundColor Cyan
Write-Host "Target Directory: $RemotePath" -ForegroundColor Gray
Write-Host "(Please enter your password when prompted)" -ForegroundColor Yellow

# 4. Execute SSH
# Note: Since we are using password auth, SSH will prompt interactively.
ssh -p $Port -t "$User@$HostIP" $RemoteCommand
