#include <iostream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif
#include "github_api.h"
#include "utils.h"

int main() {
#ifdef _WIN32
    // 设置控制台代码页为GBK
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
#endif

    std::cout << "GitHub仓库克隆工具 (CLI版本)\n";
    std::cout << "============================\n";
    
    // 获取用户名
    std::cout << "请输入GitHub用户名: ";
    std::string username;
    std::getline(std::cin, username);
    
    if (username.empty()) {
        std::cout << "用户名不能为空。\n";
        return 1;
    }
    
    // 获取用户仓库列表
    std::cout << "正在获取 " << username << " 的仓库列表...\n";
    std::vector<Repository> repos = GitHubAPI::getUserRepositories(username);
    
    if (repos.empty()) {
        std::cout << "未找到用户 " << username << " 的公开仓库。\n";
        return 1;
    }
    
    // 让用户选择仓库
    std::cout << "\n找到 " << repos.size() << " 个仓库:\n";
    for (size_t i = 0; i < repos.size(); ++i) {
        std::cout << i + 1 << ". " << repos[i].name << " (★" << repos[i].stars << ")\n";
        if (!repos[i].description.empty() && repos[i].description != "null") {
            std::cout << "   描述: " << repos[i].description << "\n";
        }
        std::cout << "   URL: " << repos[i].clone_url << "\n\n";
    }
    
    int choice;
    do {
        std::cout << "请选择要克隆的仓库 (1-" << repos.size() << ", 0取消): ";
        std::cin >> choice;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            choice = -1;
        }
        
        if (choice == 0) {
            std::cout << "操作已取消。\n";
            return 0; // 用户取消
        }
        
        if (choice < 0 || choice > (int)repos.size()) {
            std::cout << "无效选择，请重新输入。\n";
        }
    } while (choice < 1 || choice > (int)repos.size());
    
    int selectedIndex = choice - 1; // 返回0基索引
    
    // 获取目标路径
    std::cout << "请选择或输入克隆目标目录: ";
    std::string destination;
    std::cin.ignore(10000, '\n'); // 清除输入缓冲区
    std::getline(std::cin, destination);
    
    if (destination.empty()) {
        std::cout << "未选择目录，操作已取消。\n";
        return 1;
    }
    
    // 构造完整路径（仓库名作为目录）
    std::string fullPath = destination + "/" + repos[selectedIndex].name;
#ifdef _WIN32
    size_t pos = 0;
    while ((pos = fullPath.find("/", pos)) != std::string::npos) {
        fullPath.replace(pos, 1, "\\");
        pos += 1;
    }
#endif
    
    std::cout << "正在克隆 " << repos[selectedIndex].name << " 到 " << destination << " ...\n";
    
    // 执行克隆操作
    if (GitHubAPI::cloneRepository(repos[selectedIndex].clone_url, fullPath)) {
        std::cout << "仓库克隆成功！\n";
        return 0;
    } else {
        std::cout << "仓库克隆失败。\n";
        return 1;
    }
}