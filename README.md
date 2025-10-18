# GitHub 仓库克隆工具

Copyright (C) 2025 OpenSource Contributors

这是一个可以帮助你轻松下载 GitHub 用户代码仓库的小工具，它有多种使用方式：适合普通用户的图形界面版和适合技术用户的命令行版。

## 许可证

本项目采用 GNU General Public License v3.0 许可证。详情请见 [LICENSE](LICENSE) 文件。

## 这个工具有什么用？

假设你想下载某个 GitHub 用户的所有公开项目到你的电脑上，通常你需要一个一个地去找、去下载。这个工具可以帮你：
- 自动获取指定用户的所有公开项目列表
- 让你选择想要下载哪些项目
- 一键将选中的项目保存到你电脑上的指定位置

## 怎么使用这个工具？

### 方法一：图形界面版（推荐给普通用户）

这是最简单的使用方式，就像平常使用 Windows 软件一样：
1. 双击运行 `github_clone_win32.exe`
2. 在打开的窗口里输入你想下载项目的 GitHub 用户名（比如 microsoft、google 等）
3. 点击"获取仓库"按钮，稍等片刻就能看到这个用户所有的公开项目
4. 勾选你想要下载的项目
5. 选择保存到电脑上的哪个文件夹
6. 点击"克隆选中项"按钮开始下载

### 方法二：命令行版（适合熟悉命令行的用户）

如果你更喜欢用命令行，或者想批量处理，可以使用命令行版本：
```bash
github_clone_cli <用户名> <保存路径>
```

例如，要把微软的所有公开项目下载到 D:\Projects 文件夹：
```bash
github_clone_cli microsoft D:\Projects
```

### 方法三：Qt 图形界面版（功能最丰富的界面）

最新版本使用 Qt 框架开发，具有更现代、更友好的用户界面：
1. 运行 `github_clone_qt.exe`
2. 在界面上输入 GitHub 用户名
3. 点击"获取仓库"按钮获取仓库列表
4. 从列表中选择要克隆的仓库
5. 选择保存路径
6. 点击"克隆选中仓库"按钮开始下载

## Qt 图形库支持

本项目现已集成 Qt 图形库支持，可以使用 Qt 框架创建现代化的图形界面。
- 项目支持 Qt5 和 Qt6
- 使用 CMake 构建系统自动检测和链接 Qt 库

## 需要什么才能运行这个工具？

你的电脑需要满足以下条件：
- Windows 操作系统
- 能够连接互联网

## 项目包含哪些主要文件？

对于想了解项目内部结构的开发者：
- `src/main_cli.cpp` - 命令行版本主程序
- `src/main_win32.cpp` - Win32 图形界面版本主程序
- `src/main_qt.cpp` - Qt 图形界面版本主程序入口
- `src/mainwindow.h` - Qt 主窗口类头文件
- `src/mainwindow.cpp` - Qt 主窗口类实现
- `src/github_api.cpp` - 与 GitHub 网站通信的功能
- `src/utils.cpp` - 一些通用的辅助功能
- `include/github_api.h` - GitHub 功能的接口声明
- `include/utils.h` - 辅助功能的接口声明
- `resources/resource.rc` - Win32 图形界面的资源文件
- `resources/logo.ico` - 应用程序图标文件

## 备份文件

- `src/main_win32_backup.cpp` - Win32 版本的备份代码