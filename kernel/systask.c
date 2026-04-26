/*************************************************************************//**
 *****************************************************************************
 * @file   systask.c
 * @brief
 * @author Forrest Y. Yu
 * @date   2007
 *****************************************************************************
 *****************************************************************************/

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
#include "keyboard.h"
#include "proto.h"

PRIVATE int read_register(char reg_addr);
PRIVATE u32 get_rtc_time(struct time *t);

/* 简单检查回复目标是否仍存在 */
static inline int can_reply(int pid) {
	return !(proc_table[pid].p_flags & FREE_SLOT);
}

/*****************************************************************************
 *                                task_sys
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK SYS.
 *
 *****************************************************************************/
PUBLIC void task_sys()
{
	MESSAGE msg;
	struct time t;

	while (1) {
		send_recv(RECEIVE, ANY, &msg);
		int src = msg.source;

		char tmpbuf[20];
		msgtype_interpret(msg.type, tmpbuf);
		post_log2("systask: handle %s", tmpbuf);

		switch (msg.type) {
		case GET_TICKS:
			msg.RETVAL = ticks;
			if (can_reply(src)) {
				send_recv(SEND, src, &msg);
			}
			break;
		case GET_PID:
			msg.type = SYSCALL_RET;
			msg.PID = src;
			if (can_reply(src)) {
				send_recv(SEND, src, &msg);
			}
			break;
		/* 获取进程表信息 */
		case GET_PROCS: {
			struct proc_info * user_buf = (struct proc_info *)msg.BUF;
			int buf_size = msg.CNT;
			int copied = 0;
			struct proc_info info;
			int i;

			for (i = 0; i < NR_TASKS + NR_PROCS && copied < buf_size; i++) {
				struct proc * p = &proc_table[i];
				if (p->p_flags & FREE_SLOT) {
					continue;
				}

				info.pid = i;
				info.parent = p->p_parent;
				info.priority = p->priority;
				info.ticks = p->ticks;
				info.flags = p->p_flags;
				memcpy(info.name, p->name, sizeof(info.name));
				info.name[sizeof(info.name) - 1] = 0;

				phys_copy(va2la(src, user_buf + copied),
					  va2la(TASK_SYS, &info),
					  sizeof(info));
				copied++;
			}

			msg.type = SYSCALL_RET;
			msg.RETVAL = copied;
			if (can_reply(src)) {
				send_recv(SEND, src, &msg);
			}
			break;
		}
		case GET_RTC_TIME:
			msg.type = SYSCALL_RET;
			get_rtc_time(&t);
			phys_copy(va2la(src, msg.BUF),
				  va2la(TASK_SYS, &t),
				  sizeof(t));
			if (can_reply(src)) {
				send_recv(SEND, src, &msg);
			}
			break;
		default:
			panic("unknown msg type");
			break;
		}
	}
}


/*****************************************************************************
 *                                get_rtc_time
 *****************************************************************************/
/**
 * Get RTC time from the CMOS
 *
 * @return Zero.
 *****************************************************************************/
PRIVATE u32 get_rtc_time(struct time *t)
{
	t->year = read_register(YEAR);
	t->month = read_register(MONTH);
	t->day = read_register(DAY);
	t->hour = read_register(HOUR);
	t->minute = read_register(MINUTE);
	t->second = read_register(SECOND);

	if ((read_register(CLK_STATUS) & 0x04) == 0) {
		/* Convert BCD to binary (default RTC mode) */
		t->year = BCD_TO_DEC(t->year);
		t->month = BCD_TO_DEC(t->month);
		t->day = BCD_TO_DEC(t->day);
		t->hour = BCD_TO_DEC(t->hour);
		t->minute = BCD_TO_DEC(t->minute);
		t->second = BCD_TO_DEC(t->second);
	}

	t->year += 2000;

	return 0;
}

/*****************************************************************************
 *                                read_register
 *****************************************************************************/
/**
 * Read register from CMOS.
 *
 * @param reg_addr
 *
 * @return
 *****************************************************************************/
PRIVATE int read_register(char reg_addr)
{
	out_byte(CLK_ELE, reg_addr);
	return in_byte(CLK_IO);
}

