#ifndef GITHUB_CLONE_GITHUB_API_H
#define GITHUB_CLONE_GITHUB_API_H

#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#endif

struct Repository {
    std::string name;
    std::string clone_url;
    std::string description;
    int stars;
};

class GitHubAPI {
public:
    /**
     * 根据用户名获取用户的公共仓库列表
     * @param username GitHub用户名
     * @return 仓库列表
     */
    static std::vector<Repository> getUserRepositories(const std::string& username);
    
    /**
     * 克隆指定仓库到指定路径
     * @param repo_url 仓库地址
     * @param destination_path 目标路径
     * @return 是否成功
     */
    static bool cloneRepository(const std::string& repo_url, const std::string& destination_path);

private:
    /**
     * 发送HTTP GET请求
     * @param url 请求地址
     * @return 响应内容
     */
    static std::string httpGet(const std::string& url);
};

#endif //GITHUB_CLONE_GITHUB_API_H