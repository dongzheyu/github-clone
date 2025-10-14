#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include "resource.h"
#include "github_api.h"
#include "utils.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 全局变量
HWND g_hUsernameEdit = NULL;
HWND g_hFetchButton = NULL;
HWND g_hRepoList = NULL;
HWND g_hPathEdit = NULL;
HWND g_hBrowseButton = NULL;
HWND g_hCloneButton = NULL;
HWND g_hStatusLabel = NULL;
std::vector<Repository> g_repositories;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnFetchButtonClick(HWND hwndDlg);
void OnBrowseButtonClick(HWND hwndDlg);
void OnCloneButtonClick(HWND hwndDlg);
DWORD WINAPI FetchRepositoriesThread(LPVOID lpParam);
DWORD WINAPI CloneRepositoryThread(LPVOID lpParam);
void UpdateRepoList();
std::string Utf8ToGbk(const std::string& utf8Str);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // 显示对话框
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, DialogProc);

    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            // 初始化控件句柄
            g_hUsernameEdit = GetDlgItem(hwndDlg, IDC_USERNAME_EDIT);
            g_hFetchButton = GetDlgItem(hwndDlg, IDC_FETCH_BUTTON);
            g_hRepoList = GetDlgItem(hwndDlg, IDC_REPO_LIST);
            g_hPathEdit = GetDlgItem(hwndDlg, IDC_PATH_EDIT);
            g_hBrowseButton = GetDlgItem(hwndDlg, IDC_BROWSE_BUTTON);
            g_hCloneButton = GetDlgItem(hwndDlg, IDC_CLONE_BUTTON);
            g_hStatusLabel = GetDlgItem(hwndDlg, IDC_STATUS_LABEL);

            // 设置按钮初始状态
            EnableWindow(g_hCloneButton, FALSE);

            // 设置对话框标题
            SetWindowTextA(hwndDlg, "GitHub仓库克隆工具");
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_FETCH_BUTTON:
                    OnFetchButtonClick(hwndDlg);
                    break;

                case IDC_BROWSE_BUTTON:
                    OnBrowseButtonClick(hwndDlg);
                    break;

                case IDC_CLONE_BUTTON:
                    OnCloneButtonClick(hwndDlg);
                    break;

                case IDCANCEL:
                    EndDialog(hwndDlg, 0);
                    return TRUE;
            }
            break;

        case WM_USER + 1: // 自定义消息，用于更新仓库列表
            UpdateRepoList();
            return TRUE;

        case WM_USER + 2: // 自定义消息，用于更新克隆状态
            {
                bool success = (bool)wParam;
                std::string statusText, messageText, titleText;

                if (success) {
                    statusText = "仓库克隆成功！";
                    messageText = "仓库克隆成功！";
                    titleText = "成功";
                    MessageBoxA(hwndDlg, messageText.c_str(), titleText.c_str(), MB_OK | MB_ICONINFORMATION);
                } else {
                    statusText = "仓库克隆失败！";
                    messageText = "仓库克隆失败！";
                    titleText = "错误";
                    MessageBoxA(hwndDlg, messageText.c_str(), titleText.c_str(), MB_OK | MB_ICONERROR);
                }
                SetWindowTextA(g_hStatusLabel, statusText.c_str());
                EnableWindow(g_hFetchButton, TRUE);
                EnableWindow(g_hCloneButton, TRUE);
            }
            return TRUE;

        case WM_CLOSE:
            EndDialog(hwndDlg, 0);
            return TRUE;
    }

    return FALSE;
}

void OnFetchButtonClick(HWND hwndDlg) {
    // 获取用户名
    char username[256];
    GetWindowTextA(g_hUsernameEdit, username, sizeof(username));

    if (strlen(username) == 0) {
        MessageBoxA(hwndDlg, "请输入GitHub用户名", "提示", MB_OK | MB_ICONWARNING);
        return;
    }

    // 清空之前的仓库列表
    SendMessageA(g_hRepoList, LB_RESETCONTENT, 0, 0);
    g_repositories.clear();

    // 禁用按钮防止重复点击
    EnableWindow(g_hFetchButton, FALSE);
    SetWindowTextA(g_hStatusLabel, "正在获取仓库列表...");

    // 创建线程获取仓库信息
    CreateThread(NULL, 0, FetchRepositoriesThread, hwndDlg, 0, NULL);
}

DWORD WINAPI FetchRepositoriesThread(LPVOID lpParam) {
    HWND hwndDlg = (HWND)lpParam;

    // 从控件获取用户名
    char username[256];
    SendMessageA(g_hUsernameEdit, WM_GETTEXT, sizeof(username), (LPARAM)username);

    // 获取仓库列表 - 从API获取的是UTF-8编码
    std::vector<Repository> repos = GitHubAPI::getUserRepositories(username);

    // 将UTF-8编码内容转换为GBK编码以便在Windows上正确显示
    for (auto& repo : repos) {
        repo.name = Utf8ToGbk(repo.name);
        repo.description = Utf8ToGbk(repo.description);
        // clone_url 通常为ASCII编码不需要转换
    }

    // 在UI主线程中进行操作
    g_repositories = repos;

    // 发送消息通知主线程更新UI
    PostMessage(hwndDlg, WM_USER + 1, 0, 0);

    return 0;
}

void UpdateRepoList() {
    // 启用获取按钮
    EnableWindow(g_hFetchButton, TRUE);

    if (g_repositories.empty()) {
        SetWindowTextA(g_hStatusLabel, "未找到该用户的所有仓库");
        return;
    }

    // 填充仓库列表
    int validRepoCount = 0;
    for (const auto& repo : g_repositories) {
        // 过滤无效仓库（没有名称和URL的）
        if (repo.name.empty() || repo.clone_url.empty()) {
            continue;
        }

        // 使用GBK编码的字符串显示
        std::string itemText = repo.name + " (" + std::to_string(repo.stars) + ")";

        // 添加描述信息
        if (!repo.description.empty()) {
            itemText += " - " + repo.description;
        } else {
            itemText += " - 无描述";
        }

        SendMessageA(g_hRepoList, LB_ADDSTRING, 0, (LPARAM)itemText.c_str());
        validRepoCount++;
    }

    if (validRepoCount == 0) {
        SetWindowTextA(g_hStatusLabel, "未找到有效仓库");
        return;
    }

    std::string statusText = "找到 " + std::to_string(validRepoCount) + " 个仓库";
    SetWindowTextA(g_hStatusLabel, statusText.c_str());
    EnableWindow(g_hCloneButton, TRUE);
}

void OnBrowseButtonClick(HWND hwndDlg) {
    std::string path = Utils::selectDirectoryWin32();
    if (!path.empty()) {
        SetWindowTextA(g_hPathEdit, path.c_str());
    }
}

void OnCloneButtonClick(HWND hwndDlg) {
    // 检查是否选择了仓库
    int selectedIndex = (int)SendMessageA(g_hRepoList, LB_GETCURSEL, 0, 0);
    if (selectedIndex == LB_ERR) {
        MessageBoxA(hwndDlg, "请在列表中选择一个仓库", "提示", MB_OK | MB_ICONWARNING);
        return;
    }

    // 获取目标路径
    char pathBuffer[MAX_PATH];
    GetWindowTextA(g_hPathEdit, pathBuffer, MAX_PATH);

    if (strlen(pathBuffer) == 0) {
        MessageBoxA(hwndDlg, "请选择或输入目标路径", "提示", MB_OK | MB_ICONWARNING);
        return;
    }

    // 禁用按钮防止重复点击
    EnableWindow(g_hCloneButton, FALSE);
    EnableWindow(g_hFetchButton, FALSE);
    SetWindowTextA(g_hStatusLabel, "正在克隆仓库...");

    // 创建线程克隆仓库
    CreateThread(NULL, 0, CloneRepositoryThread, (LPVOID)(intptr_t)selectedIndex, 0, NULL);
}

DWORD WINAPI CloneRepositoryThread(LPVOID lpParam) {
    int selectedIndex = (int)(intptr_t)lpParam;

    // 获取目标路径
    char pathBuffer[MAX_PATH];
    SendMessageA(g_hPathEdit, WM_GETTEXT, MAX_PATH, (LPARAM)pathBuffer);

    // 构造完整路径 - 使用GBK编码的仓库名
    std::string repoName = g_repositories[selectedIndex].name; // 已经是GBK编码
    std::string fullPath = std::string(pathBuffer) + "\\" + repoName;

    // 执行克隆操作 - clone_url为UTF-8格式不需要转换
    bool success = GitHubAPI::cloneRepository(g_repositories[selectedIndex].clone_url, fullPath);

    // 发送消息通知主线程更新UI
    PostMessage(GetParent(g_hCloneButton), WM_USER + 2, (WPARAM)success, 0);

    return 0;
}

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