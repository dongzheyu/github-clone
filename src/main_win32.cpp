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

// UTF-8字符串转换函数
std::wstring Utf8ToUtf16(const std::string& utf8Str) {
    if (utf8Str.empty()) return std::wstring();
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string Utf16ToUtf8(const std::wstring& utf16Str) {
    if (utf16Str.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &utf16Str[0], (int)utf16Str.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &utf16Str[0], (int)utf16Str.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// 修复WinMain函数签名
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd) {
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);
    
    // 显示对话框
    DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN_DIALOG), NULL, DialogProc);
    
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
            SetWindowTextW(hwndDlg, L"GitHub仓库克隆工具");
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
                if (success) {
                    SetWindowTextW(g_hStatusLabel, L"仓库克隆成功！");
                    MessageBoxW(hwndDlg, L"仓库克隆成功！", L"成功", MB_OK | MB_ICONINFORMATION);
                } else {
                    SetWindowTextW(g_hStatusLabel, L"仓库克隆失败！");
                    MessageBoxW(hwndDlg, L"仓库克隆失败！", L"错误", MB_OK | MB_ICONERROR);
                }
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
    wchar_t username[256];
    GetWindowTextW(g_hUsernameEdit, username, sizeof(username) / sizeof(wchar_t));
    
    if (wcslen(username) == 0) {
        MessageBoxW(hwndDlg, L"请输入GitHub用户名", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    
    // 清空之前的仓库列表
    SendMessageW(g_hRepoList, LB_RESETCONTENT, 0, 0);
    g_repositories.clear();
    
    // 禁用按钮，防止重复点击
    EnableWindow(g_hFetchButton, FALSE);
    SetWindowTextW(g_hStatusLabel, L"正在获取仓库列表...");
    
    // 创建线程获取仓库信息
    CreateThread(NULL, 0, FetchRepositoriesThread, hwndDlg, 0, NULL);
}

DWORD WINAPI FetchRepositoriesThread(LPVOID lpParam) {
    HWND hwndDlg = (HWND)lpParam;
    
    // 获取控件上的用户名
    wchar_t username[256];
    SendMessageW(g_hUsernameEdit, WM_GETTEXT, sizeof(username) / sizeof(wchar_t), (LPARAM)username);
    
    // 将宽字符串转换为UTF-8
    std::string utf8Username = Utf16ToUtf8(username);
    
    // 获取仓库列表
    std::vector<Repository> repos = GitHubAPI::getUserRepositories(utf8Username);
    
    // 更新UI需要在主线程中完成
    g_repositories = repos;
    
    // 发送消息到主线程更新UI
    PostMessage(hwndDlg, WM_USER + 1, 0, 0);
    
    return 0;
}

void UpdateRepoList() {
    // 启用获取按钮
    EnableWindow(g_hFetchButton, TRUE);
    
    if (g_repositories.empty()) {
        SetWindowTextW(g_hStatusLabel, L"未找到该用户的公开仓库");
        return;
    }
    
    // 填充仓库列表
    for (const auto& repo : g_repositories) {
        std::wstring wname = Utf8ToUtf16(repo.name);
        std::wstring wstars = std::to_wstring(repo.stars);
        std::wstring itemText = wname + L" (⭐" + wstars + L")";
        SendMessageW(g_hRepoList, LB_ADDSTRING, 0, (LPARAM)itemText.c_str());
    }
    
    std::wstring statusText = L"找到 " + std::to_wstring(g_repositories.size()) + L" 个仓库";
    SetWindowTextW(g_hStatusLabel, statusText.c_str());
    EnableWindow(g_hCloneButton, TRUE);
}

void OnBrowseButtonClick(HWND hwndDlg) {
    std::string path = Utils::selectDirectoryWin32();
    if (!path.empty()) {
        std::wstring wpath = Utf8ToUtf16(path);
        SetWindowTextW(g_hPathEdit, wpath.c_str());
    }
}

void OnCloneButtonClick(HWND hwndDlg) {
    // 检查是否选择了仓库
    int selectedIndex = (int)SendMessageW(g_hRepoList, LB_GETCURSEL, 0, 0);
    if (selectedIndex == LB_ERR) {
        MessageBoxW(hwndDlg, L"请从列表中选择一个仓库", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    
    // 获取目标路径
    wchar_t pathBuffer[MAX_PATH];
    GetWindowTextW(g_hPathEdit, pathBuffer, MAX_PATH);
    
    if (wcslen(pathBuffer) == 0) {
        MessageBoxW(hwndDlg, L"请选择或输入目标路径", L"提示", MB_OK | MB_ICONWARNING);
        return;
    }
    
    // 禁用按钮，防止重复点击
    EnableWindow(g_hCloneButton, FALSE);
    EnableWindow(g_hFetchButton, FALSE);
    SetWindowTextW(g_hStatusLabel, L"正在克隆仓库...");
    
    // 创建线程克隆仓库
    CreateThread(NULL, 0, CloneRepositoryThread, (LPVOID)(intptr_t)selectedIndex, 0, NULL);
}

DWORD WINAPI CloneRepositoryThread(LPVOID lpParam) {
    int selectedIndex = (int)(intptr_t)lpParam;
    
    // 获取目标路径
    wchar_t pathBuffer[MAX_PATH];
    SendMessageW(g_hPathEdit, WM_GETTEXT, MAX_PATH, (LPARAM)pathBuffer);
    
    // 将宽字符串转换为UTF-8
    std::string utf8Path = Utf16ToUtf8(pathBuffer);
    
    // 构造完整路径
    std::string repoName = g_repositories[selectedIndex].name;
    std::string fullPath = utf8Path + "\\" + repoName;
    
    // 执行克隆操作
    bool success = GitHubAPI::cloneRepository(g_repositories[selectedIndex].clone_url, fullPath);
    
    // 发送消息到主线程更新UI
    PostMessage(GetParent(g_hCloneButton), WM_USER + 2, (WPARAM)success, 0);
    
    return 0;
}