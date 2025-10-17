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


# ==================== Qt UI 版本 ====================
# 查找 Qt6 库（如果没找到则查找 Qt5）
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
endif()

if (Qt6_FOUND OR Qt5_FOUND)
    # 启用自动处理 Qt 的 moc、uic 和 rcc
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)
    
    add_executable(github_clone_qt
        src/main_qt.cpp
        src/mainwindow.cpp
        src/repositorylistitem.cpp
        src/github_api.cpp
        src/utils.cpp)

    # 根据找到的 Qt 版本链接相应的库
    if (Qt6_FOUND)
        target_link_libraries(github_clone_qt 
            Qt6::Core 
            Qt6::Widgets
            wininet)
        message(STATUS "Using Qt6")
    else()
        target_link_libraries(github_clone_qt 
            Qt5::Core 
            Qt5::Widgets
            wininet)
        message(STATUS "Using Qt5")
    endif()
        
    target_include_directories(github_clone_qt PRIVATE include)
endif()

# ==================== CLion 特定配置 ====================
if (CMAKE_GENERATOR STREQUAL "Ninja")
    message(STATUS "Building with Ninja in CLion - Toolchain: ${CMAKE_CXX_COMPILER_ID}")
endif()