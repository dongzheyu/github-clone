cmake_minimum_required(VERSION 3.20)
project(GithubClone)

set(CMAKE_CXX_STANDARD 17)

# ==================== 工具链检测 ====================
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Detected MinGW toolchain")
    set(USE_MINGW TRUE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Detected Visual Studio toolchain")
    set(USE_MSVC TRUE)
endif()

# ==================== 命令行版本 ====================
add_executable(github_clone_cli
        src/main_cli.cpp
        src/github_api.cpp
        src/utils.cpp)

# ==================== 公共配置 ====================
target_include_directories(github_clone_cli PRIVATE include)
target_link_libraries(github_clone_cli wininet)

# ==================== Win32 UI 版本 ====================
if (WIN32)
    add_executable(github_clone_win32 WIN32
            src/main_win32.cpp
            src/github_api.cpp
            src/utils.cpp
            resources/resource.rc)

    if (USE_MINGW)
        target_link_libraries(github_clone_win32 comctl32 wininet)
    else()
        target_link_libraries(github_clone_win32 wininet)
    endif()

    target_include_directories(github_clone_win32 PRIVATE include)
endif()

# ==================== WinUI 3 版本 ====================
# 注意: 构建 WinUI 3 版本需要先安装 Windows App SDK
# 请使用 Visual Studio Installer 安装以下组件:
# 1. Windows App SDK (在"单个组件"选项卡中搜索)
# 2. Windows App SDK 框架包
# 3. C++ WinUI 3 模板
#
# 或者从 https://aka.ms/windowsappsdk/ 下载并安装
if (WIN32 AND USE_MSVC)
    # 启用 WinRT 组件
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    
    # 设置 Windows SDK 路径
    set(WINDOWS_SDK_ROOT "C:/Windows Kits/10")
    set(WINDOWS_SDK_VERSION "10.0.22621.0")
    set(WINDOWS_SDK_INCLUDE_DIR "${WINDOWS_SDK_ROOT}/Include/${WINDOWS_SDK_VERSION}")
    set(WINDOWS_SDK_LIB_DIR "${WINDOWS_SDK_ROOT}/Lib/${WINDOWS_SDK_VERSION}")
    
    # 根据目标架构设置库目录
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WINDOWS_SDK_ARCH "x64")
    else()
        set(WINDOWS_SDK_ARCH "x86")
    endif()
    
    # 尝试查找 Windows App SDK
    find_path(WINDOWS_APP_SDK_INCLUDE_DIR 
        NAMES Microsoft.UI.Xaml.h
        PATHS 
            "C:/Program Files/WindowsApps"
            "C:/Program Files (x86)/Microsoft SDKs/Windows Kits/10/Include"
            "${WINDOWS_SDK_INCLUDE_DIR}"
        PATH_SUFFIXES 
            "winrt"
            "10.0.22621.0/winrt"
            "10.0.22000.0/winrt"
            "10.0.19041.0/winrt"
    )
    
    add_executable(github_clone_winui3 WIN32
            src/App.xaml
            src/App.xaml.cpp
            src/MainWindow.xaml
            src/MainWindow.xaml.cpp
            src/github_api.cpp
            src/utils.cpp)

    target_include_directories(github_clone_winui3 PRIVATE include)
    
    # 添加 Windows SDK 和 WinUI 包含目录
    if(WINDOWS_APP_SDK_INCLUDE_DIR)
        target_include_directories(github_clone_winui3 PRIVATE 
            ${WINDOWS_APP_SDK_INCLUDE_DIR}
            "${WINDOWS_SDK_INCLUDE_DIR}/winrt"
            "${WINDOWS_SDK_INCLUDE_DIR}/um"
            "${WINDOWS_SDK_INCLUDE_DIR}/shared")
        message(STATUS "Found Windows App SDK at: ${WINDOWS_APP_SDK_INCLUDE_DIR}")
    else()
        target_include_directories(github_clone_winui3 PRIVATE 
            "${WINDOWS_SDK_INCLUDE_DIR}/winrt"
            "${WINDOWS_SDK_INCLUDE_DIR}/um"
            "${WINDOWS_SDK_INCLUDE_DIR}/shared")
        message(WARNING "Windows App SDK not found. Please install it from Visual Studio Installer or https://aka.ms/windowsappsdk/")
    endif()
    
    # 链接必要的 Windows 和 WinUI 库
    target_link_libraries(github_clone_winui3 
        wininet
        "${WINDOWS_SDK_LIB_DIR}/um/${WINDOWS_SDK_ARCH}/WindowsApp.lib")
    
    # 设置 WinUI 3 特定的编译选项
    target_compile_definitions(github_clone_winui3 PRIVATE 
        WIN32_LEAN_AND_MEAN
        WINRT_NO_MODULE_LOCK
        WINRT_IMPLEMENTS_PROPERTY_MANUAL_RESET_EVENT)
        
    # 设置子系统为 Windows
    set_target_properties(github_clone_winui3 PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:WINDOWS")
        
    # 设置包含目录以确保能正确找到 .g.h 文件
    target_include_directories(github_clone_winui3 PRIVATE src)
endif()

# ==================== CLion 特定配置 ====================
if (CMAKE_GENERATOR STREQUAL "Ninja")
    message(STATUS "Building with Ninja in CLion - Toolchain: ${CMAKE_CXX_COMPILER_ID}")
endif()