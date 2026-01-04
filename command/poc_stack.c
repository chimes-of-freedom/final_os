/**
 * poc_stack.c - 栈攻击 PoC（Proof of Concept）
 * 
 * 此程序故意将返回地址篡改为非法值，用于测试动态度量防护是否有效。
 */

#include "stdio.h"

int main(int argc, char *argv[])
{
    volatile unsigned int *ebp_ptr;
    volatile unsigned int *ret_addr_ptr;
    unsigned int original_ret;
    unsigned int after_tamper;
    
    printf("[PoC] Stack attack test program\n");
    
    /* 内联汇编获取当前 EBP（这是虚拟地址，相对于段基址的偏移） */
    __asm__ __volatile__ ("movl %%ebp, %0" : "=r"(ebp_ptr));
    
    /* 返回地址在 saved EBP 的下一个位置 (高地址方向) */
    ret_addr_ptr = ebp_ptr + 1;
    
    printf("EBP (virtual addr): 0x%x\n", (unsigned int)ebp_ptr);
    printf("ret_addr_ptr (virtual addr): 0x%x\n", (unsigned int)ret_addr_ptr);
    
    /* 读取原始返回地址 */
    original_ret = *ret_addr_ptr;
    printf("Original return address: 0x%x\n", original_ret);
    
    /* 篡改返回地址 */
    printf("Tampering to 0xDEADBEEF...\n");
    *ret_addr_ptr = 0xDEADBEEF;
    
    /* 读取篡改后的值确认是否成功 */
    after_tamper = *ret_addr_ptr;
    printf("After tamper: 0x%x\n", after_tamper);
    
    if (after_tamper == 0xDEADBEEF) {
        printf("Tamper SUCCESS! Calling syscall to trigger check...\n");
    } else {
        printf("Tamper FAILED! Value is 0x%x instead of 0xDEADBEEF\n", after_tamper);
    }
    
    /* 再次调用系统调用触发检查 */
    printf("After this printf, check should trigger...\n");
    
    /* 如果执行到这里，说明防护失效 */
    printf("[PoC] ERROR: Protection failed!\n");
    
    return 0;
}
