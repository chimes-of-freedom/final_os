#include "stdio.h"
#include "proto.h"

PUBLIC void free(const void *mem) {
	if (!mem) {
		return;
	}

	MESSAGE msg;
	msg.type = FREE;
	msg.BUF = mem;
	send_recv(BOTH, TASK_MM, &msg);

	assert(msg.RETVAL == SYSCALL_RET);
}
