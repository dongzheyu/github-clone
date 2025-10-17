#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <set>
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

// 声明函数
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
            
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            {
                // 处理列表框中的复选框点击
                if (HWND(lParam) == g_hRepoList) {
                    // 获取鼠标点击位置
                    POINT pt;
                    pt.x = LOWORD(wParam);
                    pt.y = HIWORD(wParam);
                    
                    // 获取列表框中的项目索引
                    int index = SendMessageA(g_hRepoList, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
                    if (!(index & 0xFFFF0000)) { // 检查是否点击在有效项目上
                        // 计算点击是否在复选框区域
                        if (pt.x >= 2 && pt.x <= 16) { // 复选框宽度为14像素，加上边距
                            // 切换复选框状态
                            DWORD itemData = SendMessageA(g_hRepoList, LB_GETITEMDATA, index, 0);
                            itemData ^= 1; // 切换最低位（选中状态）
                            SendMessageA(g_hRepoList, LB_SETITEMDATA, index, itemData);
                            InvalidateRect(g_hRepoList, NULL, TRUE); // 重绘列表框
                            
                            // 检查是否有选中的项目来更新克隆按钮状态
                            bool hasSelection = false;
                            int count = SendMessageA(g_hRepoList, LB_GETCOUNT, 0, 0);
                            for (int i = 0; i < count; i++) {
                                DWORD data = SendMessageA(g_hRepoList, LB_GETITEMDATA, i, 0);
                                if (data & 1) {
                                    hasSelection = true;
                                    break;
                                }
                            }
                            
                            char pathBuffer[MAX_PATH];
                            GetWindowTextA(g_hPathEdit, pathBuffer, MAX_PATH);
                            EnableWindow(g_hCloneButton, hasSelection && strlen(pathBuffer) > 0);
                        }
                    }
                }
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
            
        case WM_DRAWITEM:
            // 处理自定义绘制列表项（添加复选框）
            if (wParam == IDC_REPO_LIST) {
                LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
                if (lpDrawItem->itemID == -1) break;
                
                HDC hdc = lpDrawItem->hDC;
                RECT rc = lpDrawItem->rcItem;
                
                // 绘制选中状态背景
                if (lpDrawItem->itemState & ODS_SELECTED) {
                    SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    FillRect(hdc, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
                } else {
                    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
                    FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
                }
                
                // 从列表项数据中获取选中状态
                DWORD itemData = SendMessageA(g_hRepoList, LB_GETITEMDATA, lpDrawItem->itemID, 0);
                bool isChecked = (itemData & 1);
                
                // 绘制复选框
                RECT rcCheck;
                rcCheck.left = rc.left + 2;
                rcCheck.top = rc.top + 2;
                rcCheck.right = rcCheck.left + 14;
                rcCheck.bottom = rcCheck.top + 14;
                
                DrawFrameControl(hdc, &rcCheck, DFC_BUTTON, 
                    DFCS_BUTTONCHECK | 
                    (isChecked ? DFCS_CHECKED : 0));
                
                // 绘制文本
                std::string* itemText = (std::string*)lpDrawItem->itemData;
                RECT rcText = rc;
                rcText.left += 20;
                DrawTextA(hdc, itemText->c_str(), -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                
                // 绘制焦点框
                if (lpDrawItem->itemState & ODS_FOCUS) {
                    DrawFocusRect(hdc, &rc);
                }
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

    // 将UTF-8编码转换为GBK编码以便在Windows上正确显示
    for (auto& repo : repos) {
        repo.name = Utf8ToGbk(repo.name);
        repo.description = Utf8ToGbk(repo.description);
        // clone_url 通常是ASCII编码，不需要转换
    }

    // 在UI线程中更新需要的全局变量
    g_repositories = repos;

    // 发送消息通知主线程更新UI
    PostMessage(hwndDlg, WM_USER + 1, 0, 0);

    return 0;
}

void UpdateRepoList() {
    // 启用获取按钮
    EnableWindow(g_hFetchButton, TRUE);

    if (g_repositories.empty()) {
        SetWindowTextA(g_hStatusLabel, "未找到该用户仓库");
        return;
    }

    // 填充仓库列表，每个项目前面添加未选中的复选框状态
    int validRepoCount = 0;
    for (const auto& repo : g_repositories) {
        // 检查是否为有效仓库（有名称和URL）
        if (repo.name.empty() || repo.clone_url.empty()) {
            continue;
        }

        // 构造显示文本（使用GBK编码的字符串）
        std::string itemText = repo.name + " (" + std::to_string(repo.stars) + ")";

        // 添加描述信息
        if (!repo.description.empty()) {
            itemText += " - " + repo.description;
        } else {
            itemText += " - 无描述";
        }

        // 添加项目到列表
        int index = SendMessageA(g_hRepoList, LB_ADDSTRING, 0, (LPARAM)new std::string(itemText));
        // 设置项目数据，最低位为0表示未选中
        SendMessageA(g_hRepoList, LB_SETITEMDATA, index, 0);
        validRepoCount++;
    }

    if (validRepoCount == 0) {
        SetWindowTextA(g_hStatusLabel, "未找到有效仓库");
        return;
    }

    std::string statusText = "找到 " + std::to_string(validRepoCount) + " 个仓库";
    SetWindowTextA(g_hStatusLabel, statusText.c_str());
    EnableWindow(g_hCloneButton, FALSE); // 初始状态下没有选中项
}

void OnBrowseButtonClick(HWND hwndDlg) {
    std::string path = Utils::selectDirectoryWin32();
    if (!path.empty()) {
        SetWindowTextA(g_hPathEdit, path.c_str());
    }
}

void OnCloneButtonClick(HWND hwndDlg) {
    // 获取选中的仓库索引
    int count = SendMessageA(g_hRepoList, LB_GETCOUNT, 0, 0);
    std::vector<int> selectedIndices;
    
    for (int i = 0; i < count; i++) {
        // 检查每个项目的选中状态（通过检查复选框状态）
        DWORD itemData = SendMessageA(g_hRepoList, LB_GETITEMDATA, i, 0);
        if (itemData & 1) { // 最低位为1表示选中
            selectedIndices.push_back(i);
        }
    }
    
    if (selectedIndices.empty()) {
        MessageBoxA(hwndDlg, "请从列表中选择至少一个仓库", "提示", MB_OK | MB_ICONWARNING);
        return;
    }

    // 获取目标路径
    char pathBuffer[MAX_PATH];
    GetWindowTextA(g_hPathEdit, pathBuffer, MAX_PATH);

    if (strlen(pathBuffer) == 0) {
        MessageBoxA(hwndDlg, "请选择保存的目标路径", "提示", MB_OK | MB_ICONWARNING);
        return;
    }

    // 禁用按钮防止重复点击
    EnableWindow(g_hCloneButton, FALSE);
    EnableWindow(g_hFetchButton, FALSE);
    std::string status = "正在克隆 " + std::to_string(selectedIndices.size()) + " 个仓库...";
    SetWindowTextA(g_hStatusLabel, status.c_str());

    // 创建线程克隆仓库（传递选中的索引列表）
    CreateThread(NULL, 0, CloneRepositoryThread, new std::vector<int>(selectedIndices), 0, NULL);
}

DWORD WINAPI CloneRepositoryThread(LPVOID lpParam) {
    std::vector<int>* selectedIndices = (std::vector<int>*)lpParam;

    // 获取目标路径
    char pathBuffer[MAX_PATH];
    SendMessageA(g_hPathEdit, WM_GETTEXT, MAX_PATH, (LPARAM)pathBuffer);

    bool allSuccess = true;
    int clonedCount = 0;
    int totalToClone = selectedIndices->size();
    
    // 克隆每个选中的仓库
    for (int index : *selectedIndices) {
        // 获取仓库名称（已经是GBK编码）
        std::string repoName = g_repositories[index].name;
        std::string fullPath = std::string(pathBuffer) + "\\" + repoName;

        // 执行克隆操作（clone_url是UTF-8编码）
        bool success = GitHubAPI::cloneRepository(g_repositories[index].clone_url, fullPath);
        if (!success) {
            allSuccess = false;
        }
        clonedCount++;
        
        // 更新状态信息
        std::string status = "正在克隆 " + std::to_string(clonedCount) + "/" + std::to_string(totalToClone) + " 个仓库...";
        SendMessageA(GetDlgItem(GetParent(g_hPathEdit), IDC_STATUS_LABEL), WM_SETTEXT, 0, (LPARAM)status.c_str());
    }

    // 清理内存
    delete selectedIndices;

    // 发送消息通知主线程更新UI
    PostMessage(GetParent(g_hCloneButton), WM_USER + 2, (WPARAM)allSuccess, 0);

    return 0;
}

// UTF-8转GBK函数
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