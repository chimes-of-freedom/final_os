<#
    OS Project Development Workflow Controller
    Usage: .\dev.ps1 [init | connect | test]
#>

param(
    [string]$Action
)

# Get current script directory to locate utils
$ScriptRoot = $PSScriptRoot
$UtilsPath = Join-Path $ScriptRoot "utils"

# Dispatch tasks based on argument
switch ($Action) {
    "init" {
        Write-Host ">>> Initializing configuration..."
        & "$UtilsPath\init.ps1"
    }
    "connect" {
        Write-Host ">>> Connecting to server..."
        & "$UtilsPath\connect.ps1"
    }
    "test" {
        Write-Host ">>> Pulling image and testing..."
        & "$UtilsPath\test.ps1"
    }
    Default {
        Write-Host "Usage: .\dev.ps1 [init | connect | test]" -ForegroundColor Yellow
        Write-Host "  init    - Initialize config (generate sftp.json, user config)"
        Write-Host "  connect - SSH connect to server"
        Write-Host "  test    - Pull artifacts -> Decompress -> Run Bochs"
    }
}
