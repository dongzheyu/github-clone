#include <iostream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif
#include "github_api.h"
#include "utils.h"

#ifdef _WIN32
// UTF-8转GBK编码函数
std::string Utf8ToGbk(const std::string& utf8Str) {
    if (utf8Str.empty()) return utf8Str;

    // 先将UTF-8转换为宽字符
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (wideCharLength <= 0) return utf8Str;

    std::vector<WCHAR> wideStr(wideCharLength);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideStr.data(), wideCharLength);

    // 再将宽字符转换为GBK
    int gbkLength = WideCharToMultiByte(936, 0, wideStr.data(), -1, NULL, 0, NULL, NULL);
    if (gbkLength <= 0) return utf8Str;

    std::vector<char> gbkStr(gbkLength);
    WideCharToMultiByte(936, 0, wideStr.data(), -1, gbkStr.data(), gbkLength, NULL, NULL);

    return std::string(gbkStr.data(), gbkLength - 1); // -1 to exclude null terminator
}
#endif

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

    // 转换仓库信息为GBK编码以便在控制台正确显示
    std::vector<Repository> gbkRepos = repos;
    for (auto& repo : gbkRepos) {
        repo.name = Utf8ToGbk(repo.name);
        repo.description = Utf8ToGbk(repo.description);
        // clone_url 通常是ASCII，不需要转换
    }

    // 让用户选择仓库
    std::cout << "\n找到 " << gbkRepos.size() << " 个仓库:\n";
    for (size_t i = 0; i < gbkRepos.size(); ++i) {
        std::cout << i + 1 << ". " << gbkRepos[i].name << " (★" << gbkRepos[i].stars << ")\n";
        if (!gbkRepos[i].description.empty() && gbkRepos[i].description != "null") {
            std::cout << "   描述: " << gbkRepos[i].description << "\n";
        }
        std::cout << "   URL: " << repos[i].clone_url << "\n\n"; // URL保持UTF-8
    }

    int choice;
    do {
        std::cout << "请选择要克隆的仓库 (1-" << gbkRepos.size() << ", 0取消): ";
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

        if (choice < 0 || choice > (int)gbkRepos.size()) {
            std::cout << "无效选择，请重新输入。\n";
        }
    } while (choice < 1 || choice > (int)gbkRepos.size());

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

    // 构造完整路径（仓库名作为目录）- 使用原始UTF-8仓库名用于路径
    std::string fullPath = destination + "/" + repos[selectedIndex].name; // 使用原始UTF-8仓库名
#ifdef _WIN32
    size_t pos = 0;
    while ((pos = fullPath.find("/", pos)) != std::string::npos) {
        fullPath.replace(pos, 1, "\\");
        pos += 1;
    }
#endif

    std::cout << "正在克隆 " << gbkRepos[selectedIndex].name << " 到 " << destination << " ...\n";

    // 执行克隆操作 - 使用原始UTF-8 URL和路径
    if (GitHubAPI::cloneRepository(repos[selectedIndex].clone_url, fullPath)) {
        std::cout << "仓库克隆成功！\n";
        return 0;
    } else {
        std::cout << "仓库克隆失败。\n";
        return 1;
    }
}