#include "stdio.h"

#define BUFFER_SIZE 512

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("cat: Usage: cat filename\n");
		return 1;
	}

	char *filename = argv[1];
	int fd = open(filename, O_RDWR);

	if (fd == -1) {
		printf("cat: Failed to open %s\n", filename);
		return 1;
	}

	char buffer[BUFFER_SIZE + 1];
	struct stat *s = (struct stat *)buffer;
	stat(filename, s);
	if (s->st_mode != I_REGULAR) {
		printf("cat: Failed to read %s: permission denied\n", filename);
		return 1;
	}
	while (1)
	{
		int r = read(fd, buffer, BUFFER_SIZE);
		if (r == -1) {
			printf("cat: Something goes wrong when reading %s\n", filename);
		}
		if (r == 0 || r == -1)
		{

			break;
		}
		buffer[r] = 0;
		printf(buffer);
	}

	close(fd);
	return 0;
}
