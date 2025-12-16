if (!(Test-Path -Path "80m.img")) {
	Write-Host "80m.img does not exist."
	Write-Host "Extract 80m.img.gz here using `gunzip -k 80m.img.gz` before starting emulation."
	exit 1
}

bochs -f bochsrc.win -debugger
