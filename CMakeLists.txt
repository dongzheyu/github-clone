cmake_minimum_required(VERSION 3.20)
project(GithubClone)

set(CMAKE_CXX_STANDARD 17)

# 添加可执行文件
add_executable(github_clone_cli 
    src/main_cli.cpp
    src/github_api.cpp
    src/utils.cpp)

# 添加Win32 UI版本
if(WIN32)
    add_executable(github_clone_win32 WIN32
        src/main_win32.cpp
        src/github_api.cpp
        src/utils.cpp
        resources/resource.rc)
    
    target_include_directories(github_clone_win32 PRIVATE include)
    target_link_libraries(github_clone_win32 comctl32 wininet)
endif()

target_include_directories(github_clone_cli PRIVATE include)
target_link_libraries(github_clone_cli wininet)

# 检查是否在CLion中构建
if(CMAKE_GENERATOR STREQUAL "Ninja")
    # CLion使用Ninja构建时的特殊处理
    message(STATUS "Building with Ninja in CLion")
endif()