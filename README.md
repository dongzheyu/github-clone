# GitHub 仓库克隆工具

这是一个可以克隆 GitHub 用户仓库的工具，支持命令行界面和图形界面。

## 功能

- 通过用户名获取 GitHub 仓库列表
- 克隆选中的仓库到本地目录
- 支持 Win32 和 WinUI 3 两种图形界面版本
- 命令行版本支持自动化操作

## 编译要求

- C++17 或更高版本
- CMake 3.20 或更高版本
- Windows SDK (用于 Win32 和 WinUI 3 版本)
- Windows App SDK (用于 WinUI 3 版本)

## 构建说明

使用 CMake 构建项目:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### 可执行文件

构建后将生成以下可执行文件:

1. `github_clone_cli` - 命令行版本
2. `github_clone_win32` - Win32 图形界面版本
3. `github_clone_winui3` - WinUI 3 图形界面版本(需要 Windows App SDK)

## 使用方法

### 命令行版本

```bash
github_clone_cli <username> <destination_path>
```

示例:
```bash
github_clone_cli microsoft D:\Projects
```

### 图形界面版本

运行 `github_clone_win32.exe` 或 `github_clone_winui3.exe`，在界面中输入 GitHub 用户名，获取仓库列表，选择要克隆的仓库和目标路径，然后点击克隆按钮。

## WinUI 3 版本说明

WinUI 3 版本提供了现代化的用户界面，具有以下特点:

- 现代化的 Fluent Design 设计语言
- 更好的触摸和鼠标交互支持
- 支持深色/浅色主题
- 响应式布局，适应不同屏幕尺寸
- 更丰富的动画和视觉效果

注意: WinUI 3 版本需要 Windows App SDK 才能运行，但已包含在项目中。

### 构建 WinUI 3 版本的详细步骤

1. 确保使用 Visual Studio 2022 或兼容的 MSVC 工具链
2. CMake 会自动使用项目中的 Windows App SDK 包
3. 使用 CMake 构建项目（如上所示）
4. 如果遇到链接错误，可能需要手动添加 Windows App SDK 引用

## 文件说明

- `src/main_cli.cpp` - 命令行版本主程序
- `src/main_win32.cpp` - Win32 图形界面版本主程序
- `src/github_api.cpp` - GitHub API 接口实现
- `src/utils.cpp` - 工具函数实现
- `include/github_api.h` - GitHub API 接口头文件
- `include/utils.h` - 工具函数头文件
- `resources/resource.rc` - Win32 资源文件
- `src/App.xaml` - WinUI 3 应用程序定义
- `src/MainWindow.xaml` - WinUI 3 主窗口界面定义
- `src/github_clone_winui3.exe.manifest` - WinUI 3 应用程序清单
- `packages/` - Windows App SDK 包目录

## 备份文件

- `src/main_win32_backup.cpp` - Win32 版本的备份文件