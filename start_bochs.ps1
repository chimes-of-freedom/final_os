if (!(Test-Path -Path "80m.img")) {
	Write-Host "未找到 80m.img。"
	Write-Host "请先将 80m.img.gz 解压为 80m.img 后再运行。"
	exit 1
}

bochs -f bochsrc.win -debugger
