#include "type.h"
#include "stdio.h"
#include "string.h"
#include "const.h"
#include "proc.h"
#include "proto.h"

int main(int argc, char *argv[])
{
	MESSAGE msg;
	log_s buff[11];
	int i = 0;
	memset(&msg, 0, sizeof(msg));
	memset(buff, 0, sizeof(buff));

	if (argc > 3)
	{
		printf("logman: Too many arguments\n");
		goto usage;
	}
	else{
		if (argc < 2 || !strcmp(argv[1], "h")) 
		{
			goto usage;
		}
		else if (!strcmp(argv[1], "c")) 
		{
			msg.type = CLEAR_LOG;
			send_recv(SEND, TASK_LOG, &msg);
		}
		else if (!strcmp(argv[1], "r")) 
		{
			msg.type = RECENT_LOG;
			msg.BUF = buff;
			send_recv(BOTH, TASK_LOG, &msg);
			for(i = 0; i < 10; ++i)
			{
				printf("%s\n", buff[i].text);
			}
		}
		else if (!strcmp(argv[1], "s")) 
		{
			msg.type = SWITCH_LOG;
			send_recv(SEND, TASK_LOG, &msg);
		}
		else if (argc == 3 && !strcmp(argv[1], "p")) 
		{
			msg.type = GET_PID_LOG;
			msg.BUF = buff;
			msg.CNT = atoi(argv[2]);
			send_recv(BOTH, TASK_LOG, &msg);
			for(i = 0; i < 10; ++i)
			{
				printf("%s\n", buff[i].text);
			}
		}
		else if (argc == 3 && !strcmp(argv[1], "b")) 
		{
			msg.type = BLOCK_LOG;
			msg.CNT = atoi(argv[2]);
			send_recv(SEND, TASK_LOG, &msg);
		}
		else
		{
			printf("logman: Invaild arg.\n");
			goto usage;
		}
	}
	return 0;

usage:
	printf("logman: Usage\n");
	printf("    h - show help\n");
	printf("    c - clear log\n");
	printf("    r - print recent 10 logs\n");
	printf("    s - stop/restart log\n");
	printf("    p <pid> - print recent 10 logs of certain pid\n");
	printf("    b <pid> - block/unblock log from certain pid\n");
	return 1;
}