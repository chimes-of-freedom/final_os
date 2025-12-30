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

PUBLIC void msgtype_interpret(int msgtype, char* buf)
{
	switch(msgtype)
	{
		case HARD_INT:
			strcpy(buf, "HARD_INT");
			break;
		case GET_TICKS:
			strcpy(buf, "GET_TICKS");
			break;
		case GET_PID:
			strcpy(buf, "GET_PID");
			break;
		case GET_RTC_TIME:
			strcpy(buf, "GET_RTC_TIME");
			break;
		case GET_PROCS:
			strcpy(buf, "GET_PROCS");
			break;
		case OPEN:
			strcpy(buf, "OPEN");
			break;
		case CLOSE:
			strcpy(buf, "CLOSE");
			break;
		case READ:
			strcpy(buf, "READ");
			break;
		case WRITE:
			strcpy(buf, "WRITE");
			break;	
		case LSEEK:
			strcpy(buf, "LSEEK");
			break;
		case STAT:
			strcpy(buf, "STAT");
			break;
		case UNLINK:
			strcpy(buf, "UNLINK");
			break;
		case SUSPEND_PROC:	
			strcpy(buf, "SUSPEND_PROC");
			break;
		case RESUME_PROC:
			strcpy(buf, "RESUME_PROC");
			break;
		case EXEC:
			strcpy(buf, "EXEC");
			break;
		case WAIT:
			strcpy(buf, "WAIT");
			break;
		case KILL:
			strcpy(buf, "KILL");
			break;
		case FORK:
			strcpy(buf, "FORK");
			break;
		case EXIT:
			strcpy(buf, "EXIT");
			break;
		case SYSCALL_RET:
			strcpy(buf, "SYSCALL_RET");
			break;
		case POST_LOG:
			strcpy(buf, "POST_LOG");
			break;
		case DEV_OPEN:
			strcpy(buf, "DEV_OPEN");
			break;
		case DEV_CLOSE:
			strcpy(buf, "DEV_CLOSE");
			break;
		case DEV_READ:
			strcpy(buf, "DEV_READ");
			break;
		case DEV_WRITE:
			strcpy(buf, "DEV_WRITE");
			break;
		case DEV_IOCTL:
			strcpy(buf, "DEV_IOCTL");
			break;
		default:
			strcpy(buf, "....");
			break;
	}
	return;
}

PUBLIC void post_log2(const char* fmt, ...)
{
	char buf[MAX_LOG_LEN * 4], real_buf[MAX_LOG_LEN];
	int len = 0;

	memset(buf, 0, sizeof(buf));
	va_list arg = (va_list)((char*)(&fmt) + 4);
	vsprintf(buf, fmt, arg);

	len = min(strlen(buf), MAX_LOG_LEN - 1);
	strcpy(real_buf, buf);
	real_buf[len] = '\0';

	MESSAGE msg;
	memset(&msg, 0, sizeof(0));
	msg.type = POST_LOG;
	msg.CNT = len;
	msg.BUF = real_buf;

	send_recv(SEND, TASK_LOG, &msg);
}

PUBLIC void task_log()
{
	MESSAGE	log_msg, res; 
	log_s logs[MAX_LOG_CNT], *ans;

	memset(logs, 0, sizeof(logs));
	int log_cnt = 0, i = 0, enabled = 1, roll = 0, pos = 0, id = 0, base = 0;
	char black_list[2048];
	memset(black_list, 0, sizeof(black_list));
	printl("task_log start....");

	while(1)
	{
		memset(&res, 0, sizeof(res));
		send_recv(RECEIVE, ANY, &log_msg);
		base = 0;
		int msgtype = log_msg.type;
		int src = log_msg.source;
		
		switch(msgtype)
		{
			case POST_LOG:
				if(enabled == 0 || black_list[src]) break;
				phys_copy(va2la(TASK_LOG, logs[log_cnt].text),
					va2la(src, (char*)log_msg.BUF) , log_msg.CNT);
				logs[log_cnt].text[log_msg.CNT] = '\0';
				logs[log_cnt].pid = src;
				logs[log_cnt].len = log_msg.CNT;
				printl("[LOG] pid %d : \"%s\"\n", src, logs[log_cnt].text);
				log_cnt++;
				if(log_cnt >= MAX_LOG_CNT) 
				{
					log_cnt = 0;
					roll = 1;
				}
				break;
			case SWITCH_LOG:
				enabled ^= 1;
				break;
			case CLEAR_LOG:
				memset(logs, 0, sizeof(logs));
				log_cnt = roll = 0;
				break;
			case RECENT_LOG:
				ans = (log_s *)log_msg.BUF;
				for(i = 1; i <= 10; ++i)
				{
					pos = (log_cnt + MAX_LOG_CNT - i) % MAX_LOG_CNT;

					ans[i].pid = logs[pos].pid;
					phys_copy(va2la(src, ans[i].text),
						va2la(TASK_LOG, 
							logs[pos].text), logs[pos].len );
				}
				res.type = LOG_RET;
				send_recv(SEND, src, &res);
				break;
			case GET_PID_LOG:
				ans = (log_s *)log_msg.BUF;
				id = log_msg.CNT;
				for(i = 1; i <= 10; ++i)
				{
					if(base >= MAX_LOG_CNT) break;
					pos = (base + log_cnt + MAX_LOG_CNT - i) % MAX_LOG_CNT;
					
					if(logs[pos].pid != id)
					{
						i--;
						base++;
						continue;
					}
					ans[i].pid = logs[pos].pid;
					phys_copy(va2la(src, ans[i].text),
						va2la(TASK_LOG, 
							logs[pos].text), logs[pos].len );
				}
				res.type = LOG_RET;
				send_recv(SEND, src, &res);
				break;
			case BLOCK_LOG:
				black_list[log_msg.CNT] ^= 1;
				break;
		}
	}
}