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

	int fd;
	if (have_old) {
		fd = open(path, flag_create | O_RDWR);
		if (fd < 0) {
			printf("editor: Failed to open %s\n", name);
			return 1;
		}
	}

	printf("--- Editing %s", name);
	if (!have_old) {
		printf(" (new)");
	}
	printf(". You can trim tail bytes then append. ---\n");

	int old_size = have_old ? st.st_size : 0;
	int buf_cap = old_size + 4096; /* allow extra input */
	char *buf = (char*)malloc(buf_cap + 1);
	if (!buf) {
		printf("editor: No memory\n");
		if (have_old) {
			close(fd);
		}
		return 1;
	}

	int old_read = 0;
	if (have_old) {
		while (old_read < old_size) {
			int want = old_size - old_read;
			if (want > 512)
				want = 512;
			int got = read(fd, buf + old_read, want);
			if (got <= 0)
				break;
			old_read += got;
		}
		old_size = old_read;
		buf[old_size] = '\0';
		printf("%s", buf);
	}

	int del_bytes = 0;
	if (have_old) {
		printf("\n--- Bytes to delete from end (0 to keep): ");
		char del_input[32];
		int dlen = read(0, del_input, sizeof(del_input) - 1);
		if (dlen > 0) {
			del_input[dlen] = '\0';
			del_bytes = atoi(del_input);
			if (del_bytes < 0)
				del_bytes = 0;
			if (del_bytes > old_size)
				del_bytes = old_size;
		}
	}

	int keep = old_size - del_bytes;
	if (keep < 0) {
		keep = 0;
	}

	printf("--- Append text, empty line to finish. ---\n");
	int append_cap = buf_cap - keep;
	if (append_cap < 0) {
		append_cap = 0;
	}
	int append_len = read(0, buf + keep, append_cap);
	if (append_len < 0) {
		append_len = 0;
	}

	int final_len = keep + append_len;
	
	if (have_old) {
		close(fd);
		unlink(path);
	}

	fd = open(path, O_CREAT | O_RDWR);
	if (fd < 0) {
		printf("fd = %d\n", fd);
		printf("editor: Failed to reopen %s\n", name);
		free(buf, buf_cap + 1);
		return 1;
	}

	if (final_len > 0)
		write(fd, buf, final_len);

	free(buf, buf_cap + 1);
	close(fd);
	return 0;
}
