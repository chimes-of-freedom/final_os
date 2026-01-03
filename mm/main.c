/*************************************************************************//**
 *****************************************************************************
 * @file   mm/main.c
 * @brief  Orange'S Memory Management.
 * @author Forrest Y. Yu
 * @date   Tue May  6 00:33:39 2008
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "config.h"
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

PUBLIC void do_fork_test();
PUBLIC void* do_mallocfree(int mode);
PRIVATE void init_mm();

/*****************************************************************************
 *                                task_mm
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK MM.
 * 
 *****************************************************************************/
PUBLIC void task_mm()
{
	init_mm();

	while (1) {
		send_recv(RECEIVE, ANY, &mm_msg);
		int src = mm_msg.source;
		int reply = 1;

		int msgtype = mm_msg.type;

		char tmpbuf[20];
		msgtype_interpret(msgtype, tmpbuf);
		post_log2("mm: handle %s", tmpbuf);

		switch (msgtype) {
		case FORK:
			mm_msg.RETVAL = do_fork();
			break;
		case EXIT:
			do_exit(mm_msg.STATUS);
			reply = 0;
			break;
		case EXEC:
			mm_msg.RETVAL = do_exec();
			break;
		case WAIT:
			do_wait();
			reply = 0;
			break;
		case KILL:
			mm_msg.RETVAL = do_kill();
			break;
		case MALLOC:
			mm_msg.BUF = do_mallocfree(1);
			break;
		case FREE:
			mm_msg.RETVAL = (int)do_mallocfree(0);
			break;
		default:
			dump_msg("MM::unknown msg", &mm_msg);
			assert(0);
			break;
		}

		if (reply) {
			mm_msg.type = SYSCALL_RET;
			send_recv(SEND, src, &mm_msg);
		}
	}
}

/*****************************************************************************
 *                                init_mm
 *****************************************************************************/
/**
 * Do some initialization work.
 * 
 *****************************************************************************/
PRIVATE void init_mm()
{
	struct boot_params bp;
	get_boot_params(&bp);

	memory_size = bp.mem_size;

	/* print memory size */
	printl("{MM} memsize:%dMB\n", memory_size / (1024 * 1024));
}

/*****************************************************************************
 *                                alloc_mem
 *****************************************************************************/
/**
 * Allocate a memory block for a proc.
 * 
 * @param pid  Which proc the memory is for.
 * @param memsize  How many bytes is needed.
 * 
 * @return  The base of the memory just allocated.
 *****************************************************************************/
PUBLIC int alloc_mem(int pid, int memsize)
{
	assert(pid >= (NR_TASKS + NR_NATIVE_PROCS));
	if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
		panic("unsupported memory request: %d. "
		      "(should be less than %d)",
		      memsize,
		      PROC_IMAGE_SIZE_DEFAULT);
	}

	int base = PROCS_BASE +
		(pid - (NR_TASKS + NR_NATIVE_PROCS)) * PROC_IMAGE_SIZE_DEFAULT;

	if (base + memsize >= memory_size)
		panic("memory allocation failed. pid:%d", pid);

	return base;
}

/*****************************************************************************
 *                                free_mem
 *****************************************************************************/
/**
 * Free a memory block. Because a memory block is corresponding with a PID, so
 * we don't need to really `free' anything. In another word, a memory block is
 * dedicated to one and only one PID, no matter what proc actually uses this
 * PID.
 * 
 * @param pid  Whose memory is to be freed.
 * 
 * @return  Zero if success.
 *****************************************************************************/
PUBLIC int free_mem(int pid)
{
	return 0;
}

PUBLIC void* do_mallocfree(int mode)
{
	int pid = mm_msg.source;
	int size = mm_msg.CNT;
	void *buf = mm_msg.BUF;

	static unsigned char bitmap[42][102];
	static int initialized = 0;
	const unsigned int base_addr = 0x1000;
	const unsigned int end_addr = 0x4000;
	const unsigned int block_size = 0x10;
	const unsigned int total_blocks = (end_addr - base_addr) / block_size;

	unsigned int start_bit = 0, j = 0;
	if (!initialized) {
		memset(bitmap, 0, sizeof(bitmap));
		initialized = 1;
	}

	if (pid < 0 || pid >= 40) {
		return (void*)0;
	}

	if (mode == 1) {  // malloc
		if (size == 0) {
			return (void*)0;
		}
		unsigned int blocks_needed = (size + block_size - 1) / block_size;

		for (start_bit = 0; start_bit <= total_blocks - blocks_needed; ++start_bit) {
			int is_free = 1;
			for (j = 0; j < blocks_needed; ++j) {
				unsigned int bit_index = start_bit + j;
				unsigned int byte_index = bit_index / 8;
				unsigned int bit_pos = bit_index % 8;
				if (bitmap[pid][byte_index] & (1 << bit_pos)) {
					is_free = 0;
					break;
				}
			}
			if (is_free) {
				for (j = 0; j < blocks_needed; ++j) {
					unsigned int bit_index = start_bit + j;
					unsigned int byte_index = bit_index / 8;
					unsigned int bit_pos = bit_index % 8;
					bitmap[pid][byte_index] |= (1 << bit_pos);
				}
				return (void*)(base_addr + start_bit * block_size);
			}
		}
		return (void*)0;
	} else if (mode == 0) {  // free
		if (buf == (void*)0 || size == 0) {
			return (void*)0;
		}
		unsigned int ptr = (unsigned int)buf;
		if (ptr < base_addr || ptr >= end_addr || (ptr - base_addr) % block_size != 0) {
			return (void*)0;  // Invalid pointer
		}
		unsigned int start_bit = (ptr - base_addr) / block_size;
		unsigned int blocks_to_free = (size + block_size - 1) / block_size;
		if (start_bit + blocks_to_free > total_blocks) {
			return (void*)0;  // Out of bounds
		}
		for (j = 0; j < blocks_to_free; ++j) {
			unsigned int bit_index = start_bit + j;
			unsigned int byte_index = bit_index / 8;
			unsigned int bit_pos = bit_index % 8;
			bitmap[pid][byte_index] &= ~(1 << bit_pos);
		}
		return (void*)0;
	}
	return (void*)0;  // Invalid mode
}
