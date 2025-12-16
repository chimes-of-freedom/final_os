$server = if ($args -contains "--wg") { "10.0.1.2" } else { "10.201.98.97" }
$port = "6022"

Write-Host "SSH Connector" -ForegroundColor Cyan
Write-Host "Server: ${server}:${port}" -ForegroundColor Yellow
Write-Host ""

$username = Read-Host "Your username"

Write-Host ""
Write-Host "Connecting with ${username}@${server}:${port}..." -ForegroundColor Green
Write-Host "Input password if needed" -ForegroundColor Yellow
Write-Host ""

ssh -p $port ${username}@${server}
