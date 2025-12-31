#include "type.h"
#include "stdio.h"
#include "string.h"
#include "fs.h"
#include "const.h"

static inline const char *type_label(int mode)
{
	int t = mode & I_TYPE_MASK;
	switch (t) {
	case I_DIRECTORY:	return "DIR ";
	case I_REGULAR:		return "FILE";
	case I_CHAR_SPECIAL:	return "CHAR";
	case I_BLOCK_SPECIAL:	return "BLK ";
	default: 		return "UNK";
	}
}

static inline const char *calc_unit(int *size) {
	if (!(*size >> 10))	return "B  ";
	*size >>= 10;
	if (!(*size >> 10))	return "KiB";
	*size >>= 10;
	return "MiB";
}

int main(int argc, char *argv[])
{
	if (argc > 2) {
		printf("ls: Too many arguments.\n");
		goto usage;
	}
	if (argc == 2 && strcmp(argv[1], "-h")) {
		printf("ls: Invalid argument: %s\n", argv[1]);
		goto usage;
	}
	int use_unit = argc == 2;

	struct stat dir_stat;
	if (stat("/", &dir_stat) != 0) {
		printf("ls: failed to stat root directory\n");
		return 1;
	}

	int total_entries = dir_stat.st_size / DIR_ENTRY_SIZE;
	int seen = 0;

	int fd = open("/", O_RDWR);
	if (fd < 0) {
		printf("ls: failed to open root directory\n");
		return 1;
	}

	printf("TYPE INO   SIZE  NAME\n");

	while (seen < total_entries) {
		int i;
		struct dir_entry ents[16];
		int need = total_entries - seen;
		int want_bytes = min(need, (int)(sizeof(ents) / DIR_ENTRY_SIZE)) * DIR_ENTRY_SIZE;
		int bytes = read(fd, ents, want_bytes);
		if (bytes <= 0) {
			printf("ls: failed to read directory entries\n");
			close(fd);
			return 1;
		}

		int count = bytes / DIR_ENTRY_SIZE;
		seen += count;
		for (i = 0; i < count; i++) {
			char name[MAX_FILENAME_LEN + 1];
			memcpy(name, ents[i].name, MAX_FILENAME_LEN);
			name[MAX_FILENAME_LEN] = '\0';
			int len = 0;
			while (len < MAX_FILENAME_LEN && name[len])
				len++;

			if (len == 0)
				continue;

			char path[MAX_PATH];
			path[0] = '/';
			memcpy(path + 1, name, len);
			path[1 + len] = '\0';

			struct stat st;
			if (stat(path, &st) != 0)
				continue; /* 非有效条目，跳过 */

			int show_size = st.st_size;
			if (use_unit) {
				const char *show_unit = calc_unit(&show_size);
				printf("%s %3d %4d%s %s\n",
				       type_label(st.st_mode),
				       st.st_ino,
				       show_size,
				       show_unit,
				       name);
			} else {
				printf("%s %3d %7d %s\n",
				       type_label(st.st_mode),
				       st.st_ino,
				       show_size,
				       name);
			}

		}
	}

	close(fd);
	return 0;

usage:
	printf("Usage: ls [-h]\n");
	return 0;
}
