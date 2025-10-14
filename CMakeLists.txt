cmake_minimum_required(VERSION 3.20)
project(GithubClone)

set(CMAKE_CXX_STANDARD 17)

# 设置Windows App SDK路径（使用项目中的packages目录）
set(WINDOWS_APP_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.WindowsAppSDK.1.5.240404000")

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
if (WIN32 AND USE_MSVC)
    add_executable(github_clone_winui3 WIN32
            src/App.xaml
            src/App.xaml.cpp
            src/MainWindow.xaml
            src/MainWindow.xaml.cpp
            src/github_api.cpp
            src/utils.cpp
            src/github_clone_winui3.exe.manifest)

    target_include_directories(github_clone_winui3 PRIVATE include)
    
    # 添加Windows App SDK包含目录
    target_include_directories(github_clone_winui3 PRIVATE 
        ${WINDOWS_APP_SDK_PATH}/include
        ${WINDOWS_APP_SDK_PATH}/include/winrt)
    
    # 链接Windows App SDK库
    target_link_libraries(github_clone_winui3 
        wininet
        ${WINDOWS_APP_SDK_PATH}/lib/win10-x64/Microsoft.WindowsAppRuntime.lib
        ${WINDOWS_APP_SDK_PATH}/lib/native/win10-x64/Microsoft.UI.Dispatching.lib)
    
    # 设置WinUI 3特定的编译选项
    target_compile_definitions(github_clone_winui3 PRIVATE WIN32_LEAN_AND_MEAN)
    
    # 添加必要的编译定义
    target_compile_definitions(github_clone_winui3 PRIVATE 
        WINRT_NO_MODULE_LOCK
        WINRT_IMPLEMENTS_PROPERTY_MANUAL_RESET_EVENT)
        
    # 设置子系统为Windows
    set_target_properties(github_clone_winui3 PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif()

# ==================== CLion 特定配置 ====================
if (CMAKE_GENERATOR STREQUAL "Ninja")
    message(STATUS "Building with Ninja in CLion - Toolchain: ${CMAKE_CXX_COMPILER_ID}")
endif()