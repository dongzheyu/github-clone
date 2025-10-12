#include "github_api.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <winreg.h>
#include <shellapi.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
std::string GitHubAPI::httpGet(const std::string& url) {
    HINTERNET hInternet = InternetOpenA("GithubClone", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        return "";
    }

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "";
    }

    std::string response;
    char buffer[4096];
    DWORD bytesRead;

    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        response.append(buffer, bytesRead);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return response;
}
#else
std::string GitHubAPI::httpGet(const std::string& url) {
    std::string command = "curl -s \"" + url + "\"";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";

    char buffer[4096];
    std::string result;
    while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}
#endif

std::vector<Repository> GitHubAPI::getUserRepositories(const std::string& username) {
    std::vector<Repository> repos;
    std::string url = "https://api.github.com/users/" + username + "/repos?per_page=100";
    
    std::string response = httpGet(url);
    if (response.empty()) {
        std::cerr << "无法连接到GitHub API\n";
        return repos;
    }

    // 简单解析JSON响应
    // 注意：这是一个简化的JSON解析器，实际项目中建议使用专业库如nlohmann/json
    size_t pos = 0;
    while ((pos = response.find("\"name\":", pos)) != std::string::npos) {
        Repository repo;
        
        // 提取仓库名
        size_t name_start = response.find("\"", pos + 7) + 1;
        size_t name_end = response.find("\"", name_start);
        repo.name = response.substr(name_start, name_end - name_start);
        
        // 提取clone_url
        size_t clone_pos = response.find("\"clone_url\":", pos);
        size_t clone_start = response.find("\"", clone_pos + 12) + 1;
        size_t clone_end = response.find("\"", clone_start);
        repo.clone_url = response.substr(clone_start, clone_end - clone_start);
        
        // 提取描述
        size_t desc_pos = response.find("\"description\":", pos);
        if (desc_pos != std::string::npos) {
            if (response.substr(desc_pos + 14, 4) == "null") {
                repo.description = "无描述";
            } else {
                size_t desc_start = response.find("\"", desc_pos + 14) + 1;
                size_t desc_end = response.find("\"", desc_start);
                repo.description = response.substr(desc_start, desc_end - desc_start);
            }
        } else {
            repo.description = "无描述";
        }
        
        // 提取星标数
        size_t star_pos = response.find("\"stargazers_count\":", pos);
        if (star_pos != std::string::npos) {
            size_t star_start = star_pos + 19;
            size_t star_end = response.find(",", star_start);
            std::string star_str = response.substr(star_start, star_end - star_start);
            // 移除可能的空格
            star_str.erase(std::remove_if(star_str.begin(), star_str.end(), ::isspace), star_str.end());
            if (!star_str.empty()) {
                repo.stars = std::stoi(star_str);
            } else {
                repo.stars = 0;
            }
        } else {
            repo.stars = 0;
        }
        
        repos.push_back(repo);
        pos = clone_end;
    }
    
    return repos;
}

bool GitHubAPI::cloneRepository(const std::string& repo_url, const std::string& destination_path) {
    if (!Utils::isGitInstalled()) {
        std::cerr << "错误：未找到Git，请先安装Git。\n";
        return false;
    }

#ifdef _WIN32
    // Windows下使用cmd执行git命令
    std::string command = "git clone \"" + repo_url + "\" \"" + destination_path + "\"";
    
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // 创建可修改的命令行字符串
    std::vector<char> cmdLine(command.begin(), command.end());
    cmdLine.push_back('\0');
    
    if (!CreateProcessA(NULL, cmdLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return false;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return exitCode == 0;
#else
    // Unix-like系统使用system调用
    std::string command = "git clone \"" + repo_url + "\" \"" + destination_path + "\"";
    int result = system(command.c_str());
    return WEXITSTATUS(result) == 0;
#endif
}