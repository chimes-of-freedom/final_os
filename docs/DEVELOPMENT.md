# 极简开发指南

## 环境配置

- Bochs：3.0 版本，安装除 Linux Demo 外都勾上，并配置安装目录到系统环境变量。
- 开发环境：编译环境上服务器，写代码用 VS Code + C/C++ 插件，不要改 `.vscode/` 中代码格式化相关配置。
- 服务器目录挂载：SSHFS-Win Manager，注意看仓库的 README，注意如果改了其依赖的 SSHFS-Win 的安装路径则需要在 SSHFS-Win Manager 的 Settings 中修改路径。

## Git 规范

- 分支命名：`[类型]/[名字]-[模块]-[具体内容]`，比如 `feature/phz-schedule-logic`，或 `fix/nevermind-fs-wr`。
- commit message：`[类型]: [概括改动]`，比如 ``feat: 增加 `ls` 命令``，或 `chore: 修订文档关于文件系统的描述`，若需详述则分行写无序列表。

## 编码规范

- 就一个要求，尽量和 Orange OS 源码保持一致，比如 snake case，括号分行，多用宏少用硬编码。
- 对于源码不统一的部分，遵循常见的规范，比如对于 `for` 等带括号的语句，关键字和括号间有空格，而函数名和参数列表间则无空格。

## 开发工作流

1. 启动 SSHFS-Win Manager，SFTP 挂载远程目录。
2. VS Code 打开项目，运行 `connect_to_server.ps1`，SSH 登录服务器。
3. 在远程终端拉 `main` 分支到服务器，拉新分支开始开发。
4. 写完代码后在远程终端 `make image` 编译并写镜像。
5. 在本地终端运行 `start_bochs.ps1`，启动 Bochs 调试模式。
6. 测试完毕后 `commit` 提交代码，`push` 推送分支。
7. 上网页创建 PR，红灯就回到本地重新拉 `main` 然后 `git merge main`，解决冲突，直到 PR 按钮变绿，提交。

## 注意事项

1. Git 相关操作在本地和服务器上做都可以，但推荐在服务器上做，不然本机可能会把 LF 转成 CRLF。
2. 要时刻分清楚自己在用哪个终端，是 SSH 还是 PowerShell。
3. SSH 登录后进入的是家目录，要切换到 `final_os` 下再做编译和 Git 相关操作。
