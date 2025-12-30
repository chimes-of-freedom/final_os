#include "stdio.h"
#include "string.h"
#include "fs.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("rm: Usage: rm filename\n");
		return 1;
	}

	const char *input = argv[1];
	int len = strlen(input);
	if (len == 0) {
		printf("rm: Filename cannot be empty\n");
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

	if (unlink(path) != 0) {
		printf("rm: Failed to remove %s\n", name);
		return 1;
	}

	return 0;
}
