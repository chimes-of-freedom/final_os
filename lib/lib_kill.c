#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

int lib_kill(int pid, int forced)
{
	MESSAGE msg;

	msg.type	= KILL;
	msg.KILL_PID	= pid;
	msg.KILL_SIG	= forced ? SIG_KILL : SIG_TERM;
	msg.KILL_FORCE	= forced;

	if (send_recv(BOTH, TASK_MM, &msg) || msg.RETVAL != 0) {
		printf("kill: Failed to kill pid %d\n", pid);
		return 1;
	}
	return 0;
}