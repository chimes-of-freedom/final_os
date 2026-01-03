#include "stdio.h"
#include "proto.h"

PUBLIC void free(void *mem, unsigned int size) {
	if (!mem) {
		return;
	}

	MESSAGE msg;
	msg.type = FREE;
	msg.BUF = mem;
	msg.CNT = size;
	send_recv(BOTH, TASK_MM, &msg);

	assert(msg.type == SYSCALL_RET);
}
