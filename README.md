# Final OS

《Orange'S：一个操作系统的实现》教材中实现的教学系统 *Orange OS* 的改进版本，基于附赠光盘中 `chapter10/e` 下的源码构建。

## 分支区别

- `main`：原始版本，拥有非安全相关的提交，但未针对 `command/poc_*` 的攻击进行相应的防护。
- `protection`：带有安全补丁的版本，同时定期将 `main` 合并到本分支。

## 环境要求

- 项目构建环境：Ubuntu 14.04 i386
- 构建环境依赖：`gcc make nasm mtools binutils zip unzip libc6-dev bc`
- 项目开发、测试环境：Windows 10/11 (x64) and Bochs 3.0

> Ubuntu 14.04 的 `apt` 命令不如 `apt-get` 完善，尽量避免使用。

## 如何构建

在构建环境下在项目根目录执行 `remote_scripts/dev image`，该脚本会完成如下操作：

1. 将 `80m.imag.tar.gz` 解压为 `80.img` 硬盘镜像；
2. 在项目根目录下运行 `make image` 以构建系统并将 `boot.bin`、`loader.bin` 和 `kernel.bin` 写入 `a.img` 软盘镜像；
3. 在 `command/` 目录下运行 `make install` 以构建系统命令、打包命令、将包写入 `80m.img`；
4. 将镜像打包为 `artifacts.zip`（用于开发工作流）。

## 如何运行

1. 拉取 `artifacts.zip` 到测试环境。
2. 解压后在目录内执行 `bochs -q -f bochsrc.win` 以指定使用 `bochs.win` 配置文件启动 Bochs。

> 如果没有配置 Bochs 的环境变量，也可以使用其 GUI 加载配置文件。

## 如何开发

开发工作流采用“构建、开发分离”以及脚本自动化的思路，因此需要准备 2 套环境并根据情况修改自动化脚本。

### 准备构建环境

构建环境采用 Ubuntu 14.04 i386 以满足原 Orange OS 老旧的构建环境要求。

1. 安装 Ubuntu 14.04 Server i386，并完成静态 IP、软件源换源、启用 ssh 等配置。
2. 安装工具：`sudo apt-get update && sudo apt-get install gcc make nasm mtools binutils zip unzip libc6-dev bc`
3. 在家目录下创建 `projects/final_os` 目录作为构建的工作目录。

### 准备开发环境

1. 下载并安装 VS Code。
2. 安装 SFTP 插件，并设置 VS Code 默认终端为 PowerShell。
3. 克隆项目后在 VS Code 中打开项目。
4. 修改 `utils/init.ps1`：将如下变量修改为想要使用的 IP 地址。

    ```powershell
    # 提供 2 种方式连接构建环境：内网 IP 或 VPN
    # 本质上是多个可选地址，运行时会询问是否使用 VPN 以决定启用哪个地址
    # 可根据需求重写此处逻辑
    $IntranetIP = "server_intranet_host"    # 构建环境的内网 IP 地址
    $VpnIP = "server_vpn_address"           # 构建环境的 VPN 地址（如果有，否则可忽略）
    $Port = <server_ssh_port>               # 构建环境的 SSH 端口
    ```

5. 执行 `.\dev.ps1 init` 以完成初始化配置。根据提示输入构建环境登录用户信息即可。
6. 按下 `Ctrl+Shift+P` 快捷键，输入 `SFTP`，选择 `SFTP: Upload Project` 以将项目必要的内容上传到构建环境指定目录。
7. 如何构建：执行 `.\dev.ps1 connect` 以连接构建环境，然后通过 `remote_scripts/dev image` 构建。
8. 如何切换连接构建环境方式：重新执行 `.\dev.ps1 init` 即可。

> - `.\dev.ps1 init` 输入的用户信息以明文形式存储于 `dev\de_config.json` 下。
> - 如果没看到 `SFTP: Upload Project`，重启 VS Code 即可。
> - 如果执行 `remote_scripts/dev image` 提示无权限，则需要手动设置脚本执行权限：`chmod +x remote_scripts/dev`
> - 如果希望直接使用脚本名 `dev image` 执行构建，请自行配置环境变量。

### 准备测试环境

默认的测试环境是 Bochs on Windows 10/11 x64，若使用 Linux 作为测试环境则需要手动拉取 `artifacts.zip` 进行测试。

1. 从官网获取 Bochs 3.0 并安装，安装选项保持默认即可。
2. 配置 Bochs 的环境变量。
3. 在项目根目录下执行 `.\dev.ps1 test`，脚本会自动从构建环境拉取必要的资源到本地并启动 Bochs。

### 启用 GDB 调试

项目默认生成并拉取带符号表的内核 `kernel.elf`，配置好 GDB 和 Bochs 即可使用 GDB 远程调试。

#### 编译支持 GDB Stub 的 Bochs

1. 获取 Bochs 3.0 源码。
2. 配置 MSYS2 并安装最新 MINGW 工具链：`mingw-w64-ucrt-x86_64-toolchain`。
3. 在 UCRT64 环境下进入 Bochs 源码目录并编译支持 GDB Stub 和 win32 的版本：`./configure --enable-gdb-stub --with-win32 && make -j$(nproc)`
4. 用编译得到的 `bochs.exe` 替换已安装 Bochs 安装目录下的 `bochs.exe`。

#### 配置 VS Code 的调试功能

VS Code 通过 `.vscode/launch.json` 配置调试任务。项目已经配置好 `launch.json`，按下 F5 即可启动调试。也可以根据自身情况修改。

#### 修改 Bochs 配置文件

Bochs 通过指定的配置文件决定是否启用 GDB Stub 功能；若启用，则会在启动模拟器后等待 GDB 的连接。

打开项目根目录下的 `bochsrc.win`，去掉最后一行的注释以启用 GDB Stub：

```plaintext
# open listener for gdbstub on port 1234
gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0
```

#### 开启调试

最后执行 `.\dev.ps1 test`，并在 VS Code 中按下 F5 以启动调试。

## 如何贡献

参见 [DEVELOPMENT.md](docs/DEVELOPMENT.md)。

## 代码审查记录

参见 [REVIEW.md](docs/REVIEW.md)。

## Todo List

参见 [TODO.md](docs/TODO.md)。

## Acknowledgements

This project is based on the educational operating system **Orange'S**
developed by Yu Yuan for the book
"Orange'S: An Operating System Implementation".

## License

This project is licensed under the GNU General Public License Version 2 (GPLv2).

See the LICENSE file for the full license text.

## Copyright

This project is derived from the Orange'S operating system source code
from the book "Orange'S: An Operating System Implementation".

Original Orange'S source code  
Copyright (c) Yu Yuan

Modifications and additional code in this repository  
Copyright (c) 2026 chimes-of-freedom
