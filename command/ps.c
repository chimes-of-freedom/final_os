#include "type.h"
#include "stdio.h"
#include "string.h"
#include "const.h"
#include "proc.h"
#include "proto.h"

#define MAX_PROCS (NR_TASKS + NR_PROCS)

static const char* state_from_flags(int flags)
{
	if (flags & FREE_SLOT) return "FREE";
	if (flags & HANGING) return "HANG";
	if (flags & WAITING) return "WAIT";
	if (flags & SENDING) return "SEND";
	if (flags & RECEIVING) return "RECV";
	return "RUN ";
}

int main(int argc, char *argv[])
{
	struct proc_info table[MAX_PROCS];
	MESSAGE msg;
	(void)argc;
	(void)argv;
	memset(&msg, 0, sizeof(msg));

	msg.type = GET_PROCS;
	msg.BUF = table;
	msg.CNT = MAX_PROCS;

	if (send_recv(BOTH, TASK_SYS, &msg) != 0) {
		printf("ps: failed to query processes\n");
		return 1;
	}

	int count = msg.RETVAL;
	printf("PID  PPID PRI TICKS FLAGS STATE NAME\n");
	int i;
	for (i = 0; i < count; i++) {
		struct proc_info * p = &table[i];
		int parent = (p->parent == NO_TASK) ? -1 : p->parent;
		printf("%3d  %4d %3d %5d 0x%02x  %s  %s\n",
		       p->pid,
		       parent,
		       p->priority,
		       p->ticks,
		       p->flags & 0xFF,
		       state_from_flags(p->flags),
		       p->name);
	}

	return 0;
}
