#include "stdio.h"
#include "fs.h"
#include "const.h"
#include "proto.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("touch: Usage: touch filename\n");
		return 1;
	}

	const char *input = argv[1];
	int len = strlen(input);
	if (len == 0) {
		printf("touch: Filename cannot be empty\n");
		return 1;
	}

	int copy_len = min(len, MAX_FILENAME_LEN);

	char name[MAX_FILENAME_LEN + 1];
	int i;
	for (i = 0; i < copy_len; i++)
		name[i] = input[i];
	name[copy_len] = '\0';

	char path[MAX_PATH];
	path[0] = '/';
	strcpy(path + 1, name);
	path[copy_len + 1] = '\0';

	int fd = open(path, O_RDWR);
	if (fd >= 0) {
		close(fd);
		return 0;
	}

	fd = open(path, O_CREAT | O_RDWR);
	if (fd < 0) {
		printf("touch: Failed to create %s\n", name);
		return 1;
	}

	close(fd);
	return 0;
}
