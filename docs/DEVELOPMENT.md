# 极简开发指南

## 环境配置

- Bochs：3.0 版本，安装除 Linux Demo 外都勾上，并配置安装目录到系统环境变量。
- 开发环境：编译环境上服务器，写代码用 VS Code + C/C++ 插件 + SFTP 插件，不要改 `.vscode/` 中代码格式化相关配置。

## Git 规范

- 分支命名：`[类型]/[名字]-[模块]-[具体内容]`，比如 `feature/phz-schedule-logic`，或 `fix/nevermind-fs-wr`。
- commit message：`[类型]: [概括改动]`，比如 ``feat: 增加 `ls` 命令``，或 `chore: 修订文档关于文件系统的描述`，若需详述则分行写无序列表。

## 编码规范

- 就一个要求，尽量和 Orange OS 源码保持一致，比如 snake case，括号分行，多用宏少用硬编码。
- 对于源码不统一的部分，遵循常见的规范，比如对于 `for` 等带括号的语句，关键字和括号间有空格，而函数名和参数列表间则无空格。
- **多写注释，不然根本看不懂。**

## 开发工作流

以下是开发工作流：

1. 【首次】克隆项目到 Windows 并用 VS Code 打开。
2. 【首次】VS Code 安装 SFTP 插件后 Windows 上运行脚本 `dev.ps1 init` 并根据引导进行初始化配置，以生成对应的用户凭证文件和 `sftp.json`。
3. **`Ctrl+Shift+P` 输入 `Upload`，找到 `SFTP: Upload Project`，执行以将项目同步到服务器。** 如果找不到选项，请重启 VS Code。
4. 拉取主分支：`git checkout main && git pull origin main`，然后在 Windows 上进行开发。代码相关的改动将会同步到服务器 `~/projects/final_os` 下。SFTP 插件只监视变化。
5. Windows 上执行 `dev.ps1 connect` 以 SSH 连接到服务器，切换到 `projects/final_os` 下后使用 `dev image` 进行编译：
   1. `dev` 执行 `tar xzf 100m.img.tar.gz` 解压硬盘镜像为 `100.img`；
   2. `dev` 执行 `make image` 编译项目并写入软盘/硬盘镜像；
   3. `dev` 执行 `zip 100m.img.zip 100m.img` 压缩 `100m.img`。
   4. 如果是其他的选项，例如 `dev clean`，则只会执行 `make clean`。
6. Windows 上执行 `dev.ps1 test` 进行镜像拉取、解压和测试：
   1. `dev.ps1` 通过 `scp` 将服务器上的 `a.img` 和 `100m.img.zip` 拉取到本地；
   2. `dev.ps1` 解压 `100m.img.zip` 为 `100.img`；
   3. `dev.ps1` 执行 `bochs -q -f bochsrc.win -debugger` 启动 Bochs 调试。
   4. 注意：如果需要 GDB 远程调试则需要在 Bochs 配置文件中开启对应功能，然后在 Bochs 启动后按下 F5。
7. Windows 上测试完成后进行 Commit，具体步骤为：
   1. `git checkout -b tag/content`
   2. `git add .`；
   3. `git commit -m 'tag: 做了 xxx'`；
   4. `git push origin tag/content`；
   5. Pull Request；
   6. 待 PR 被接收后 `git checkout main && git pull origin main && git branch -d tag/content`。

**【注意】**

1. 以下叙述用“Windows”代指本地机器。
2. 标注 `【首次】` 的项代表只需要在环境配置时执行。
3. **所有代码编写、测试以及 Git 相关操作都在 Windows 上进行**；服务器只做编译工作。
4. **SFTP 插件只会在 VS Code 打开文件的时候监视文件变化，因此必须在每次打开 VS Code 以及拉取分支后手动执行 `SFTP: Upload Project` 以将项目同步到服务器。**

## 注意事项

1. 要时刻分清楚自己在用哪个终端，是 SSH 的 Bash 还是本地的 PowerShell。
2. SSH 登录后进入的直接就是 `~/projects/final_os`。
3. `settings.json` 配置了 VS Code 默认不显示 `*.bin`、`*.o` 等编译中间产物，可在文件资源管理器查看。
