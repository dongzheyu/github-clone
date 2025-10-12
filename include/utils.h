#ifndef GITHUB_CLONE_UTILS_H
#define GITHUB_CLONE_UTILS_H

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

class Utils {
public:
    /**
     * 检查系统是否安装了Git
     * @return 如果安装了Git返回true，否则返回false
     */
    static bool isGitInstalled();
    
    /**
     * 在控制台显示仓库列表并让用户选择
     * @param repos 仓库列表
     * @return 用户选择的仓库索引，如果取消则返回-1
     */
    static int selectRepositoryCLI(const std::vector<struct Repository>& repos);
    
    /**
     * 获取用户输入的字符串
     * @param prompt 提示信息
     * @return 用户输入的内容
     */
    static std::string getInput(const std::string& prompt);
    
    /**
     * 获取用户选择的目录路径
     * @param prompt 提示信息
     * @return 用户选择的目录路径
     */
    static std::string selectDirectory(const std::string& prompt);
    
#ifdef _WIN32
    /**
     * Win32版本的目录选择对话框
     * @return 用户选择的目录路径
     */
    static std::string selectDirectoryWin32();
#endif
};

#endif //GITHUB_CLONE_UTILS_H