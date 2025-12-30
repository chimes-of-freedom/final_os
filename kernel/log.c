#include "type.h"
#include "config.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
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
	printl(real_buf);
	send_recv(SEND, TASK_LOG, &msg);
}

PUBLIC void task_log()
{
	MESSAGE	log_msg; 
	char tmp_log[MAX_LOG_CNT][MAX_LOG_LEN];
	memset(tmp_log, 0, sizeof(tmp_log));
	int log_cnt = 0, i = 0;

	printl("task_log start....");

	while(1)
	{
		send_recv(RECEIVE, ANY, &log_msg);
		int msgtype = log_msg.type;
		int src = log_msg.source;

		switch(msgtype)
		{
			case POST_LOG:
				phys_copy(va2la(TASK_LOG, tmp_log[log_cnt]),
					va2la(src, (char*)log_msg.BUF) , log_msg.CNT);
				printl("[LOG] from pid{%d} with message \"%s\" (%d bytes)\n", 
					src, 
					tmp_log[log_cnt], 
					log_msg.CNT);
				log_cnt++;
				break;
			case GET_LOG:
				for(i = 0; i < log_cnt; ++i)
				{
					printl("%s\n", tmp_log);
				}
				break;
		}
	}
}