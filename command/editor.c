#include "stdio.h"
#include "string.h"
#include "fs.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("editor: Usage: editor filename\n");
		return 1;
	}

	char *input = argv[1];
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

	struct stat st;
	int have_old = (stat(path, &st) == 0);
	int flag_create = have_old ? 0 : O_CREAT;

	int fd = open(path, flag_create | O_RDWR);
	if (fd < 0) {
		printf("editor: Failed to open %s\n", name);
		return 1;
	}

	/* 追加到文件末尾：通过读完已有内容推动 fd_pos 到末尾 */
	if (have_old && st.st_size > 0) {
		char skipbuf[128];
		int left = st.st_size;
		while (left > 0) {
			int chunk = left > (int)sizeof(skipbuf) ? (int)sizeof(skipbuf) : left;
			int r = read(fd, skipbuf, chunk);
			if (r <= 0)
				break;
			left -= r;
		}
	}

	printf("> Editing %s (append). Enter text, empty line to finish.\n", name);

	char buf[128];
	while (1) {
		int r = read(0, buf, sizeof(buf) - 1);
		if (r <= 0)
			break; /* 空行或读取失败：结束编辑 */
		buf[r] = '\0';

		write(fd, buf, r);
	}

	close(fd);
	return 0;
}
