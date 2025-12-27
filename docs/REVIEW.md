# Orange OS 历史版本代码审查

## `chapter9/a`

- `kernel/hd.c`：`hd_identify()` 中 `cmd` 的部分成员未被初始化，可能造成未定义行为。

## `chapter9/c`

- `kernel/hd.c`：`hd_rdwr()` 中 175 行、 179-181 行等多处假设主设备号永远为 `0`，但又在 178 行等多处同时考虑主设备号为 `0` 和 `1` 的情况。`hd_ioctl()` 有类似问题。
- `kernel/hd.c`：`hd_rdwr()` 中 197 行假设 `p->CNT` 即 `bytes_left` 不一定为扇区整数倍，但又在 196 行和 210 行推翻了这一假设。若情况为前者，则 `while` 条件可能一直为真。
- `include/const.h`：`ROOT_DEV` 指的是 `hd2a` 的主+次设备号，而 `MESSAGE.DEVICE` 指的是次设备号。设备号相关命名都有类似问题。建议用更清晰的命名。

## `chapter9/d`

- `include/fs.h`：`SUPER_BLOCK_SIZE` 这个宏改成 `(sizeof(struct super_block) - sizeof(int))` 可移植性会更好。`INODE_SIZE` 类似。
- `fs/main.c`：`mkfs()` 中 100 行算 `sb.nr_smap_sects` 应该加余数而不是加 1，尽管这样做通常也不会出问题。

## `chapter9/e`

- `fs/main.c`：`mkfs()` 中保留 0 号 inode 作为无效 inode 号，但未为其在 inodes 数组中保留空间，导致 inode 号和 inode 在数组中的索引错位。使用时须注意。
- `fs/main.c`：`mkfs()` 中 `sb.nr_smap_sects` 根据分区扇区数计算而来，但实际上 sector map 映射的是数据扇区即 `sb.n_1st_sect`，并且保留 bit 0。

区分 `mkfs()` 涉及的结构和对应索引关系的教材原话：

> 读者想必也发现了，搞清楚 i-node 号和 inode-map 中的位置是件容易迷糊的事情，我们不妨再来理顺一下：
>
> 对根目录文件而言，i-node 号为 1（`ROOT_INODE` 定义为 1），在 inode-map 中占用第 1 位（从 0 开始数），具体 i-node 数据位于 `inode_array[0]`。于是，第 M 号 i-node 在 inode-map 中占用第 M 位（从 0 开始数），具体 i-node 数据位于 `inode_array[M-1]`。
>
> 我们还可以知道，`inode_array` 中的第 M 项（从 0 开始数）对应第 M+1 号 i-node 以及 inode-map 中的第 M+1 位（从 0 开始数）。
>
> 与此类似，根目录区的开始扇区即为第 `sb.n_1st_sect` 扇区，占用 sector-map 中的第 1 位（从 0 开始数）。于是，第 M 扇区（以本分区的开始扇区为 0 扇区）对应 sector-map 中的第 `(M-super_block.n_1st_sect+1)` 位。同时 sector-map 中的第 M 位对应第 `(M-1+super_block.n_1st_sect)` 扇区。

## `chapter11/c`

- 已知 bug：TTY0 输入任意一个字符系统即死锁。
- 已知 bug：TTY1/TTY2 输入 `pwd` 命令即 `assert(p_who_wanna_receive-》p_flags == RECEIVING) failed`。