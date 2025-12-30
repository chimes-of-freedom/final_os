#include "stdio.h"
#include "string.h"
#include "fs.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("editor: Usage: editor filename\n");
		return 1;
	}

	const char *input = argv[1];
	int len = strlen(input);
	if (len == 0) {
		printf("editor: Filename cannot be empty\n");
		return 1;
	}

	int copy_len = len;
	if (copy_len > MAX_FILENAME_LEN)
		copy_len = MAX_FILENAME_LEN;

	char name[MAX_FILENAME_LEN + 1];
	memcpy(name, input, copy_len);
	name[copy_len] = '\0';

	char path[MAX_PATH];
	path[0] = '/';
	memcpy(path + 1, name, copy_len);
	path[copy_len + 1] = '\0';

	/* 清空旧文件，确保编辑结果覆盖 */
	struct stat st;
	if (stat(path, &st) == 0) {
		if (unlink(path) != 0) {
			printf("editor: Failed to prepare %s\n", name);
			return 1;
		}
	}

	int fd = open(path, O_CREAT | O_RDWR);
	if (fd < 0) {
		printf("editor: Failed to open %s\n", name);
		return 1;
	}

	printf("> Editing %s. Enter text, empty line to finish.\n", name);

	char buf[128];
	while (1) {
		printf("> ");
		int r = read(0, buf, sizeof(buf) - 1);
		if (r <= 0)
			break; /* 空行或读取失败：结束编辑 */
		buf[r] = '\0';

		write(fd, buf, r);
	}

	close(fd);
	return 0;
}
