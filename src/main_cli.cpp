#include <iostream>
#include <string>
#include <fcntl.h>
#include <io.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "github_api.h"
#include "utils.h"

int main() {
#ifdef _WIN32
    // 设置控制台代码页为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // 设置控制台输入/输出为宽字符模式
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#endif

    // 使用宽字符串输出中文
    std::wcout << L"GitHub仓库克隆工具 (CLI版本)\n";
    std::wcout << L"============================\n";
    
    // 获取用户名
    std::wcout << L"请输入GitHub用户名: ";
    std::wstring username;
    std::getline(std::wcin, username);
    
    if (username.empty()) {
        std::wcout << L"用户名不能为空。\n";
        return 1;
    }
    
    // 将wstring转换为string
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, username.c_str(), -1, NULL, 0, NULL, NULL);
    std::string username_utf8(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, username.c_str(), -1, &username_utf8[0], size_needed, NULL, NULL);
    
    // 获取用户仓库列表
    std::wcout << L"正在获取 " << username << L" 的仓库列表...\n";
    std::vector<Repository> repos = GitHubAPI::getUserRepositories(username_utf8);
    
    if (repos.empty()) {
        std::wcout << L"未找到用户 " << username << L" 的公开仓库。\n";
        return 1;
    }
    
    // 让用户选择仓库
    std::wcout << L"\n找到 " << repos.size() << L" 个仓库:\n";
    for (size_t i = 0; i < repos.size(); ++i) {
        std::wstring wname(repos[i].name.begin(), repos[i].name.end());
        std::wcout << i + 1 << L". " << wname << L" (⭐" << repos[i].stars << L")\n";
        if (!repos[i].description.empty() && repos[i].description != "null") {
            std::wstring wdesc(repos[i].description.begin(), repos[i].description.end());
            std::wcout << L"   描述: " << wdesc << L"\n";
        }
        std::wstring wurl(repos[i].clone_url.begin(), repos[i].clone_url.end());
        std::wcout << L"   URL: " << wurl << L"\n\n";
    }
    
    int choice;
    do {
        std::wcout << L"请选择要克隆的仓库 (1-" << repos.size() << L", 0取消): ";
        std::wcin >> choice;
        
        if (std::wcin.fail()) {
            std::wcin.clear();
            std::wcin.ignore(10000, L'\n');
            choice = -1;
        }
        
        if (choice == 0) {
            std::wcout << L"操作已取消。\n";
            return 0; // 用户取消
        }
        
        if (choice < 0 || choice > (int)repos.size()) {
            std::wcout << L"无效选择，请重新输入。\n";
        }
    } while (choice < 1 || choice > (int)repos.size());
    
    int selectedIndex = choice - 1; // 返回0基索引
    
    // 获取目标路径
    std::wcout << L"请选择或输入克隆目标目录: ";
    std::wstring destination;
    std::wcin.ignore(10000, L'\n'); // 清除输入缓冲区
    std::getline(std::wcin, destination);
    
    if (destination.empty()) {
        std::wcout << L"未选择目录，操作已取消。\n";
        return 1;
    }
    
    // 将wstring转换为string
    size_needed = WideCharToMultiByte(CP_UTF8, 0, destination.c_str(), -1, NULL, 0, NULL, NULL);
    std::string destination_utf8(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, destination.c_str(), -1, &destination_utf8[0], size_needed, NULL, NULL);
    
    // 构造完整路径（仓库名作为目录）
    std::string fullPath = destination_utf8 + "/" + repos[selectedIndex].name;
#ifdef _WIN32
    size_t pos = 0;
    while ((pos = fullPath.find("/", pos)) != std::string::npos) {
        fullPath.replace(pos, 1, "\\");
        pos += 1;
    }
#endif
    
    std::wcout << L"正在克隆 " << std::wstring(repos[selectedIndex].name.begin(), repos[selectedIndex].name.end()) 
               << L" 到 " << destination << L" ...\n";
    
    // 执行克隆操作
    if (GitHubAPI::cloneRepository(repos[selectedIndex].clone_url, fullPath)) {
        std::wcout << L"仓库克隆成功！\n";
        return 0;
    } else {
        std::wcout << L"仓库克隆失败。\n";
        return 1;
    }
}