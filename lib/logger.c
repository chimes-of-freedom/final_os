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

PUBLIC void post_log(const char* fmt, ...)
{
	char buf[MAX_LOG_LEN * 10], real_buf[MAX_LOG_LEN];
	int len = 0;
	va_list arg = (va_list)((char*)(&fmt) + 4);
	sprintf(buf, fmt, arg);

	len = min(strlen(buf), MAX_LOG_LEN - 1);
	strcpy(real_buf, buf);
	real_buf[len] = '\0';

	MESSAGE msg;
	memset(&msg, 0, sizeof(0));
	msg.type = POST_LOG;
	msg.CNT = len;
	msg.BUF = real_buf;
	printf("%s", real_buf);
	send_recv(SEND, TASK_LOG, &msg);
}