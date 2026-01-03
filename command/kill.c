#include "stdio.h"
#include "const.h"
#include "string.h"
#include "proto.h"

void spintest() {
	printf("\nspinning in spintest, waiting to be killed...\n");
	while(1)
		;
}

/**
 * @brief 结束指定进程，格式：`kill ([-9] pid)|test`
 * @note 底层暂未区分 `SIG_KILL` 和 `SIG_TERM`，都视为强制结束进程
 * @related `do_kill()`
 */
int main(int argc, char *argv[]) {
	if (argc > 3)
	{
		printf("kill: Too many arguments\n");
		goto usage;
	}
	if (argc == 3) {
		if (strcmp(argv[1], "-9")) {
			printf("kill: Wrong argument: %s\n", argv[1]);
			goto usage;
		}
		if (!isnum(argv[2])) {
			printf("kill: Wrong argument: %s\n", argv[2]);
			goto usage;
		}
	}
	/* `kill` 命令测试 */
	if (argc == 2 && !strcmp(argv[1], "test")) {
		spintest();
		return 0;
	}
	if (argc == 2 && !isnum(argv[1])) {
		printf("kill: Wrong argument: %s\n", argv[1]);
		goto usage;
	}
	if (argc == 1) {
		printf("kill: Too few argument(s)\n");
		goto usage;
	}

	int forced = argc == 3;
	int pid = argc == 2 ? atoi(argv[1]) : atoi(argv[2]);
	int self = getpid();

	if (pid == self) {
		printf("kill: Refusing to kill myself\n");
		return 1;
	}

	return lib_kill(pid, forced);

usage:
	printf("kill: Usage: kill [-9] pid\n");
	return 1;
}
