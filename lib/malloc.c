#include "stdio.h"
#include "type.h"
#include "proto.h"

PUBLIC void *malloc(const int size) {
	if (size <= 0) {
		return (void *)0;
	}

	MESSAGE msg;
	msg.type = MALLOC;
	msg.CNT = size;
	send_recv(BOTH, TASK_MM, &msg);
	
	assert(msg.type == SYSCALL_RET);
	return msg.BUF;
}