# GitHub仓库克隆工具

这是一个跨平台的GitHub仓库克隆工具，包含CLI命令行版本和Win32图形界面版本。

## 功能特性

- 根据GitHub用户名获取该用户的所有公开仓库
- 显示仓库名称、星标数和描述信息
- 选择要克隆的仓库
- 选择克隆目标位置
- 使用系统安装的Git进行仓库克隆

## 编译说明

使用CMake进行项目配置和构建：

```bash
mkdir build
cd build
cmake ..
make  # Linux/macOS
# 或者在Windows上使用Visual Studio打开生成的解决方案
```

### Windows平台

在Windows上，项目会生成两个可执行文件：
1. `github_clone_cli.exe` - 命令行版本
2. `github_clone_win32.exe` - Win32图形界面版本

### Linux/macOS平台

在Unix-like系统上，项目会生成一个可执行文件：
1. `github_clone_cli` - 命令行版本

## 使用说明

### CLI版本

1. 运行 `github_clone_cli` 程序
2. 输入要查询的GitHub用户名
3. 程序会列出该用户的所有公开仓库
4. 输入要克隆的仓库序号
5. 选择或输入目标目录
6. 程序会自动克隆选定的仓库

### Win32版本 (仅Windows)

1. 运行 `github_clone_win32.exe` 程序
2. 在文本框中输入GitHub用户名
3. 点击"获取仓库"按钮
4. 从列表中选择要克隆的仓库
5. 选择或输入目标目录
6. 点击"克隆选中仓库"按钮
7. 等待克隆完成

## 系统要求

- Git必须已安装并在系统PATH中可用
- Windows系统需要支持Win32 API
- Linux/macOS系统需要支持curl命令

## 项目结构

```
├── CMakeLists.txt          # CMake构建配置
├── include/                # 头文件目录
│   ├── github_api.h        # GitHub API接口
│   ├── utils.h             # 工具函数
│   └── resource.h          # Win32资源定义
├── src/                    # 源代码目录
│   ├── github_api.cpp      # GitHub API实现
│   ├── utils.cpp           # 工具函数实现
│   ├── main_cli.cpp        # CLI版本主程序
│   └── main_win32.cpp      # Win32版本主程序
├── resources/              # Win32资源文件
│   └── resource.rc         # 资源定义文件
└── README.md               # 本说明文件
```

## 注意事项

1. 本工具仅支持克隆公开仓库
2. 需要确保系统已安装Git并添加到PATH环境变量
3. 在使用Win32版本时，确保目标目录有写入权限
4. GitHub API有请求频率限制，避免频繁请求