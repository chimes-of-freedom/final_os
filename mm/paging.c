#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "fs.h"
#include "string.h"
#include "global.h"
#include "proto.h"

#define PAGE_DIR_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024

#define PAGE_TABLE_AREA_END (PAGE_TABLE_BASE + PAGE_TABLE_ENTRIES * PAGE_SIZE)

#define PDE_INDEX(v) (((v) >> 22) & 0x3FF)
#define PTE_INDEX(v) (((v) >> 12) & 0x3FF)

PRIVATE u32 next_free_data = 0x00A00000;
PRIVATE u32 next_free_table = PAGE_TABLE_BASE + 3 * PAGE_SIZE;
PRIVATE u32 phys_limit;

/* Exported helpers */
PUBLIC void* clone_kernel_pde();
PUBLIC void map_range_identity(u32 cr3_phys, u32 linear_start, u32 bytes, u32 flags);

PRIVATE u32 alloc_table_frame()
{
	if (next_free_table + PAGE_SIZE > PAGE_TABLE_AREA_END) {
		panic("alloc_pages: no table frames");
	}

	u32 p = next_free_table;
	next_free_table += PAGE_SIZE;
	return p;
}

PRIVATE u32 alloc_data_frame()
{
	if (phys_limit == 0) {
		if (memory_size == 0) {
			struct boot_params bp;
			get_boot_params(&bp);
			memory_size = bp.mem_size;
		}
		phys_limit = (u32)memory_size & PAGE_MASK;
		if (phys_limit < next_free_data + PAGE_SIZE) {
			panic("alloc_pages: mem too small");
		}
	}

	if (next_free_data + PAGE_SIZE > phys_limit) {
		panic("alloc_pages: OOM");
	}

	u32 p = next_free_data;
	next_free_data += PAGE_SIZE;
	return p;
}

/* Clone current kernel PDE region into a fresh page directory for a new proc. */
PUBLIC void* clone_kernel_pde()
{
	/* allocate a new page directory page */
	u32 new_pd = alloc_table_frame();
	memset((void*)new_pd, 0, PAGE_SIZE);

	/* copy only low kernel/shared PDEs (loader built first 3)
	 * NOTE: called from ring1 (MM task), so avoid privileged mov cr3;
	 * kernel page directory is at PAGE_DIR_BASE and identity-mapped. */
	u32 *cur_pd = (u32*)PAGE_DIR_BASE;
	int i;
	for (i = 0; i < 3; i++) {
		((u32*)new_pd)[i] = cur_pd[i];
	}

	return (void*)new_pd;
}

/* Map a linear range identically to same physical addresses (page aligned up). */
PUBLIC void map_range_identity(u32 cr3_phys, u32 linear_start, u32 bytes, u32 flags)
{
	u32 la = linear_start & PAGE_MASK;
	u32 end = (linear_start + bytes + PAGE_SIZE - 1) & PAGE_MASK;
	u32 *pd = (u32*)(cr3_phys & PAGE_MASK);
	int is_kernel_pd = ((cr3_phys & PAGE_MASK) == PAGE_DIR_BASE);

	for (; la < end; la += PAGE_SIZE) {
		u32 pde_idx = PDE_INDEX(la);
		u32 pte_idx = PTE_INDEX(la);

		if (!(pd[pde_idx] & PG_P)) {
			u32 pt = alloc_table_frame();
			memset((void*)pt, 0, PAGE_SIZE);
			pd[pde_idx] = (pt & PAGE_MASK) | PG_P | PG_RWW | PG_USU;
		}
		/**
		 * 对于内核的 PDE2，其指向的页表映射了 8~12MB 的空间
		 * 但对于用户进程，10~12MB 的映射要另作他用
		 * 因此不能和内核共享 PDE2 指向的页表
		 */
		else if (!is_kernel_pd && pd[pde_idx] >= PAGE_TABLE_BASE && pd[pde_idx] < PAGE_TABLE_AREA_END) {
			/* assume kernel tables are in this area; clone to keep user isolation */
			u32 pt_old = pd[pde_idx] & PAGE_MASK;
			u32 pt_new = alloc_table_frame();
			memcpy((void*)pt_new, (void*)pt_old, PAGE_SIZE);
			pd[pde_idx] = (pt_new & PAGE_MASK) | (pd[pde_idx] & 0xFFF);
		}

		u32 *pt = (u32*)(pd[pde_idx] & PAGE_MASK);
		if (!(pt[pte_idx] & PG_P)) {
			pt[pte_idx] = (la & PAGE_MASK) | PG_P | (flags & (PG_USU|PG_RWW));
		}
	}
}

PUBLIC void alloc_pages(int err_code, int eip, int cs, int eflags)
{
	u32 fault_addr;
	__asm__ __volatile__("mov %%cr2, %0" : "=r"(fault_addr));

	/* Only handle not-present faults; other causes panic for visibility */
	if (err_code & 0x1) {
		panic("PF prot err cr2=%x err=%x eip=%x cs=%x", fault_addr, err_code, eip, cs);
	}

	u32 aligned = fault_addr & PAGE_MASK;
	u32 pde_idx = PDE_INDEX(aligned);
	u32 pte_idx = PTE_INDEX(aligned);

	u32 cr3;
	__asm__ __volatile__("mov %%cr3, %0" : "=r"(cr3));

	u32 *page_dir = (u32*)(cr3 & PAGE_MASK);

	if (!(page_dir[pde_idx] & PG_P)) {
		u32 new_pt = alloc_table_frame();
		memset((void*)new_pt, 0, PAGE_SIZE);
		page_dir[pde_idx] = (new_pt & PAGE_MASK) | PG_P | PG_RWW | PG_USU;
	}

	u32 *page_table = (u32*)(page_dir[pde_idx] & PAGE_MASK);

	if (!(page_table[pte_idx] & PG_P)) {
		u32 new_page = alloc_data_frame();
		page_table[pte_idx] = (new_page & PAGE_MASK) | PG_P | PG_RWW | PG_USU;
		__asm__ __volatile__("invlpg (%0)" :: "r"(aligned) : "memory");
		memset((void*)aligned, 0, PAGE_SIZE);
	}
}
