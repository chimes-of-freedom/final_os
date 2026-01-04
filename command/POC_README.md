# 漏洞 PoC 测试程序

> 本目录包含针对 Orange'S OS 结构性安全漏洞的概念验证程序。
> 所有 PoC 仅使用 `lib/` 下的标准接口，不直接调用内核接口。

---

## PoC #1: 文件系统缺乏访问控制 (`poc_fs`)

### 漏洞信息

| 项目 | 内容 |
|------|------|
| **漏洞类型** | 缺失访问控制模型 |
| **影响范围** | 整个文件系统 |
| **严重程度** | 高危 |

### 漏洞描述

Orange'S 文件系统**完全没有实现访问控制机制**：

1. **无用户模型**：没有 UID/GID 概念
2. **无权限位**：没有 rwx 权限控制
3. **无所有权**：文件没有属主信息
4. **无访问检查**：`do_open()`、`do_unlink()` 等操作不检查调用者身份

### 攻击场景

```
攻击者进程                     受害者
    |                           |
    |-- open("/echo", O_RDWR) --|  
    |-- write(malicious_code) --|  覆盖 echo 命令
    |-- close() ----------------|
    |                           |
    |                      用户执行 echo
    |                           |
    |<---- 恶意代码执行 --------|
```

### 测试方法

```bash
# 模式 0：信息收集 - 列出可访问的文件
poc_fs 0

# 模式 1：读取攻击 - 读取可执行文件内容
poc_fs 1

# 模式 2：覆写攻击演示
poc_fs 2

# 模式 3：删除攻击演示
poc_fs 3
```

### 相关代码位置

- `fs/open.c` - `do_open()` 无权限检查
- `fs/link.c` - `do_unlink()` 无权限检查
- `fs/read_write.c` - `do_rdwt()` 无权限检查

### protection 分支修复逻辑

- `do_open()` 增加系统可执行文件访问封锁：非 INIT/FS/特定允许 PID（shell 等，排除 10、11）无法打开核心命令 `ls/ps/logman/cat/echo/pwd/editor/kill/poc_elf/poc_fs/rm/touch`，无论读或写，直接返回失败。[fs/open.c](../fs/open.c)
- 影响：
    - PoC #1 针对这些可执行文件的读/写/删攻击将失败（`open` 返回 -1）。
    - 文件系统仍无通用权限模型：对未在封锁清单内的文件，任意进程仍可读写/删除，风险部分缓解但未根除。

---

## PoC #2: ELF 加载无完整性校验 (`poc_elf`)

### 漏洞信息

| 项目 | 内容 |
|------|------|
| **漏洞类型** | 缺失代码完整性验证 |
| **影响范围** | 所有可执行文件 |
| **严重程度** | 高危 |

### 漏洞描述

Orange'S 在执行程序时**完全不验证可执行文件完整性**：

1. **无文件保护**：任何进程可修改磁盘上的可执行文件
2. **无 Magic 校验**：`do_exec()` 直接解析 ELF 头，不验证 `0x7F ELF` 魔数
3. **无完整性校验**：`exec()` 不验证 ELF 文件是否被篡改

### 攻击场景

```
攻击者进程                     受害者
    |                           |
    |-- open("/echo", O_RDWR) --|  
    |-- write(corrupted_e_entry)|  篡改 ELF e_entry 为 0xDEADBEEF
    |-- close() ----------------|
    |                           |
    |                      用户执行 echo
    |                           |
    |<---- exec() 加载篡改代码 -|
    |<-- 程序崩溃/执行恶意代码--|
```

### 测试方法

```bash
# 模式 0：信息收集 - 列出可篡改的可执行文件
poc_elf 0

# 模式 1：攻击演示 - 篡改 echo 程序的 e_entry（入口点）
# 将入口点改为 0xDEADBEEF，程序执行时会跳转到无效地址
poc_elf 1

# 模式 2：验证攻击 - 尝试执行被篡改的程序
# 会 fork 子进程尝试 exec，预期子进程崩溃
poc_elf 2
```

### 攻击原理

`mm/exec.c` 中的 `do_exec()` 函数直接将文件内容当作 `Elf32_Ehdr` 解析：

```c
Elf32_Ehdr* elf_hdr = (Elf32_Ehdr*)(mmbuf);
eip = elf_hdr->e_entry;  // 不检查 magic，直接使用
```

篡改 `e_entry`（偏移 0x18）为无效地址 `0xDEADBEEF`，程序启动时 CPU 跳转到该地址导致崩溃。

### 相关代码位置

- `mm/exec.c` - `do_exec()` 不验证文件完整性
- `fs/open.c` - 不检查可执行文件的写权限
- `fs/read_write.c` - 允许任意进程写入任意文件

### protection 分支修复逻辑

- `do_exec()` 在加载前增加完整性校验：
    - 拒绝小于 ELF 头的文件并清零缓冲区后再读入，避免残留数据伪造校验通过。
    - 校验 ELF 魔数、程序头表边界，以及每个段的 `p_offset + p_filesz` 不得越过文件末尾；不合法即返回失败（避免篡改的入口被执行）。
    - 代码位置：[mm/exec.c](../mm/exec.c)
- 预期 PoC 影响：`poc_elf` 模式 1/2 在 protection 分支应因校验失败被拒绝执行，不再跳转至篡改地址。

---

## PoC #3: 栈返回地址完整性缺失 (`poc_stack`)

### 漏洞信息

| 项目 | 内容 |
|------|------|
| **漏洞类型** | 缺失栈保护/控制流完整性 |
| **影响范围** | 所有用户态进程 |
| **严重程度** | 高危 |

### 漏洞描述

用户态调用栈上的返回地址可被任意篡改，缺少任何防护：

1. **无栈 Canaries**：编译/运行时未检测返回地址破坏
2. **无控制流完整性**：CPU 将跳转到篡改后的地址
3. **易于复现**：任意进程可写自身栈帧并劫持返回

### 测试方法

```bash
# 直接运行（无参数）
poc_stack
```

预期现象：

1. 程序打印当前 EBP、返回地址，并将返回地址改写为 `0xDEADBEEF`
2. 在 `main` 返回时，CPU 尝试跳转到非法地址，应触发异常/崩溃
3. 若屏幕出现 `[PoC] ERROR: Protection failed!` 且进程继续退出，说明缺乏栈完整性防护

### 攻击原理

`poc_stack` 直接修改保存的返回地址：

```c
__asm__ __volatile__("movl %%ebp, %0" : "=r"(ebp_ptr));
ret_addr_ptr = ebp_ptr + 1;
*ret_addr_ptr = 0xDEADBEEF;  // 返回时跳转到非法地址
```

当函数返回时，CPU 会从栈中弹出被篡改的地址，控制流被劫持，导致异常或执行攻击者指定的地址。

### 相关代码位置

- `command/poc_stack.c` - 用户态直接篡改返回地址，无任何检测

### protection 分支修复逻辑

- 每次 `send_recv` 返回给用户态前都会触发栈完整性检查：如果返回地址或栈指针越界，直接 panic 阻断控制流劫持。[kernel/proc.c](../kernel/proc.c)
- `do_stack_check()` 遍历用户态 EBP 链，校验每层返回地址是否落在进程代码段范围，并检查 EBP/ESP 越界；失败则记录 `bad_ret_addr`/`bad_ebp` 后终止。[kernel/proc.c](../kernel/proc.c)
- 新增 `stack_check_result` 结构用于报告检查结果。[include/sys/proc.h](../include/sys/proc.h)
- 预期 PoC 影响：`poc_stack` 在 protection 分支应触发内核 panic（检测到非法返回地址），无法安静返回用户态。
