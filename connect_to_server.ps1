$server = if ($args -contains "--wg") { "10.0.1.2" } else { "10.201.98.97" }
$port = "6022"

Write-Host "SSH连接配置" -ForegroundColor Cyan
Write-Host "服务器: ${server}:${port}" -ForegroundColor Yellow
Write-Host ""

$username = Read-Host "请输入用户名"

Write-Host ""
Write-Host "正在连接到 ${username}@${server}:${port}..." -ForegroundColor Green
Write-Host "请在提示时输入密码" -ForegroundColor Yellow
Write-Host ""

ssh -p $port ${username}@${server}
