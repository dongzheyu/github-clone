/*
 * GitHub仓库克隆工具
 * Copyright (C) 2025 OpenSource Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

// ȫ�ֱ���
HWND g_hUsernameEdit = NULL;
HWND g_hFetchButton = NULL;
HWND g_hRepoList = NULL;
HWND g_hPathEdit = NULL;
HWND g_hBrowseButton = NULL;
HWND g_hCloneButton = NULL;
HWND g_hStatusLabel = NULL;
std::vector<Repository> g_repositories;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnFetchButtonClick(HWND hwndDlg);
void OnBrowseButtonClick(HWND hwndDlg);
void OnCloneButtonClick(HWND hwndDlg);
DWORD WINAPI FetchRepositoriesThread(LPVOID lpParam);
DWORD WINAPI CloneRepositoryThread(LPVOID lpParam);
void UpdateRepoList();
std::string Utf8ToGbk(const std::string& utf8Str);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // ��ʼ��ͨ�ÿؼ�
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // ��ʾ�Ի���
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, DialogProc);

    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG:
            // ��ʼ���ؼ�����
            g_hUsernameEdit = GetDlgItem(hwndDlg, IDC_USERNAME_EDIT);
            g_hFetchButton = GetDlgItem(hwndDlg, IDC_FETCH_BUTTON);
            g_hRepoList = GetDlgItem(hwndDlg, IDC_REPO_LIST);
            g_hPathEdit = GetDlgItem(hwndDlg, IDC_PATH_EDIT);
            g_hBrowseButton = GetDlgItem(hwndDlg, IDC_BROWSE_BUTTON);
            g_hCloneButton = GetDlgItem(hwndDlg, IDC_CLONE_BUTTON);
            g_hStatusLabel = GetDlgItem(hwndDlg, IDC_STATUS_LABEL);

            // ���ð�ť��ʼ״̬
            EnableWindow(g_hCloneButton, FALSE);

            // ���öԻ�������
            SetWindowTextA(hwndDlg, "GitHub�ֿ���¡����");
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
                // �����б����еĸ�ѡ������
                if (HWND(lParam) == g_hRepoList) {
                    // ��ȡ��������λ��
                    POINT pt;
                    pt.x = LOWORD(wParam);
                    pt.y = HIWORD(wParam);
                    
                    // ��ȡ�б����е���Ŀ����
                    int index = SendMessageA(g_hRepoList, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
                    if (!(index & 0xFFFF0000)) { // �����Ƿ���������Ч��Ŀ��
                        // ���������Ƿ��ڸ�ѡ������
                        if (pt.x >= 2 && pt.x <= 16) { // ��ѡ������Ϊ14���أ����ϱ߾�
                            // �л���ѡ��״̬
                            DWORD itemData = SendMessageA(g_hRepoList, LB_GETITEMDATA, index, 0);
                            itemData ^= 1; // �л�����λ��ѡ��״̬��
                            SendMessageA(g_hRepoList, LB_SETITEMDATA, index, itemData);
                            InvalidateRect(g_hRepoList, NULL, TRUE); // �ػ��б���
                            
                            // �����Ƿ���ѡ�е���Ŀ�����¿�¡��ť״̬
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

        case WM_USER + 1: // �Զ�����Ϣ�����ڸ��²ֿ��б�
            UpdateRepoList();
            return TRUE;

        case WM_USER + 2: // �Զ�����Ϣ�����ڸ��¿�¡״̬
            {
                bool success = (bool)wParam;
                std::string statusText, messageText, titleText;

                if (success) {
                    statusText = "�ֿ���¡�ɹ���";
                    messageText = "�ֿ���¡�ɹ���";
                    titleText = "�ɹ�";
                    MessageBoxA(hwndDlg, messageText.c_str(), titleText.c_str(), MB_OK | MB_ICONINFORMATION);
                } else {
                    statusText = "�ֿ���¡ʧ�ܣ�";
                    messageText = "�ֿ���¡ʧ�ܣ�";
                    titleText = "����";
                    MessageBoxA(hwndDlg, messageText.c_str(), titleText.c_str(), MB_OK | MB_ICONERROR);
                }
                SetWindowTextA(g_hStatusLabel, statusText.c_str());
                EnableWindow(g_hFetchButton, TRUE);
                EnableWindow(g_hCloneButton, TRUE);
            }
            return TRUE;
            
        case WM_DRAWITEM:
            // �����Զ��������б�����Ӹ�ѡ����
            if (wParam == IDC_REPO_LIST) {
                LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
                if (lpDrawItem->itemID == -1) break;
                
                HDC hdc = lpDrawItem->hDC;
                RECT rc = lpDrawItem->rcItem;
                
                // ����ѡ��״̬����
                if (lpDrawItem->itemState & ODS_SELECTED) {
                    SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                    SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    FillRect(hdc, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
                } else {
                    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
                    SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
                    FillRect(hdc, &rc, GetSysColorBrush(COLOR_WINDOW));
                }
                
                // ���б��������л�ȡѡ��״̬
                DWORD itemData = SendMessageA(g_hRepoList, LB_GETITEMDATA, lpDrawItem->itemID, 0);
                bool isChecked = (itemData & 1);
                
                // ���Ƹ�ѡ��
                RECT rcCheck;
                rcCheck.left = rc.left + 2;
                rcCheck.top = rc.top + 2;
                rcCheck.right = rcCheck.left + 14;
                rcCheck.bottom = rcCheck.top + 14;
                
                DrawFrameControl(hdc, &rcCheck, DFC_BUTTON, 
                    DFCS_BUTTONCHECK | 
                    (isChecked ? DFCS_CHECKED : 0));
                
                // �����ı�
                std::string* itemText = (std::string*)lpDrawItem->itemData;
                RECT rcText = rc;
                rcText.left += 20;
                DrawTextA(hdc, itemText->c_str(), -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                
                // ���ƽ�����
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
    // ��ȡ�û���
    char username[256];
    GetWindowTextA(g_hUsernameEdit, username, sizeof(username));

    if (strlen(username) == 0) {
        MessageBoxA(hwndDlg, "������GitHub�û���", "��ʾ", MB_OK | MB_ICONWARNING);
        return;
    }

    // ����֮ǰ�Ĳֿ��б�
    SendMessageA(g_hRepoList, LB_RESETCONTENT, 0, 0);
    g_repositories.clear();

    // ���ð�ť��ֹ�ظ�����
    EnableWindow(g_hFetchButton, FALSE);
    SetWindowTextA(g_hStatusLabel, "���ڻ�ȡ�ֿ��б�...");

    // �����̻߳�ȡ�ֿ���Ϣ
    CreateThread(NULL, 0, FetchRepositoriesThread, hwndDlg, 0, NULL);
}

DWORD WINAPI FetchRepositoriesThread(LPVOID lpParam) {
    HWND hwndDlg = (HWND)lpParam;

    // �ӿؼ���ȡ�û���
    char username[256];
    SendMessageA(g_hUsernameEdit, WM_GETTEXT, sizeof(username), (LPARAM)username);

    // ��ȡ�ֿ��б� - ��API��ȡ����UTF-8����
    std::vector<Repository> repos = GitHubAPI::getUserRepositories(username);

    // ��UTF-8����ת��ΪGBK�����Ա���Windows����ȷ��ʾ
    for (auto& repo : repos) {
        repo.name = Utf8ToGbk(repo.name);
        repo.description = Utf8ToGbk(repo.description);
        // clone_url ͨ����ASCII���룬����Ҫת��
    }

    // ��UI�߳��и�����Ҫ��ȫ�ֱ���
    g_repositories = repos;

    // ������Ϣ֪ͨ���̸߳���UI
    PostMessage(hwndDlg, WM_USER + 1, 0, 0);

    return 0;
}

void UpdateRepoList() {
    // ���û�ȡ��ť
    EnableWindow(g_hFetchButton, TRUE);

    if (g_repositories.empty()) {
        SetWindowTextA(g_hStatusLabel, "δ�ҵ����û��ֿ�");
        return;
    }

    // �����ֿ��б���ÿ����Ŀǰ������δѡ�еĸ�ѡ��״̬
    int validRepoCount = 0;
    for (const auto& repo : g_repositories) {
        // �����Ƿ�Ϊ��Ч�ֿ⣨�����ƺ�URL��
        if (repo.name.empty() || repo.clone_url.empty()) {
            continue;
        }

        // ������ʾ�ı���ʹ��GBK�������ַ�����
        std::string itemText = repo.name + " (" + std::to_string(repo.stars) + ")";

        // ����������Ϣ
        if (!repo.description.empty()) {
            itemText += " - " + repo.description;
        } else {
            itemText += " - ������";
        }

        // ������Ŀ���б�
        int index = SendMessageA(g_hRepoList, LB_ADDSTRING, 0, (LPARAM)new std::string(itemText));
        // ������Ŀ���ݣ�����λΪ0��ʾδѡ��
        SendMessageA(g_hRepoList, LB_SETITEMDATA, index, 0);
        validRepoCount++;
    }

    if (validRepoCount == 0) {
        SetWindowTextA(g_hStatusLabel, "δ�ҵ���Ч�ֿ�");
        return;
    }

    std::string statusText = "�ҵ� " + std::to_string(validRepoCount) + " ���ֿ�";
    SetWindowTextA(g_hStatusLabel, statusText.c_str());
    EnableWindow(g_hCloneButton, FALSE); // ��ʼ״̬��û��ѡ����
}

void OnBrowseButtonClick(HWND hwndDlg) {
    std::string path = Utils::selectDirectoryWin32();
    if (!path.empty()) {
        SetWindowTextA(g_hPathEdit, path.c_str());
    }
}

void OnCloneButtonClick(HWND hwndDlg) {
    // ��ȡѡ�еĲֿ�����
    int count = SendMessageA(g_hRepoList, LB_GETCOUNT, 0, 0);
    std::vector<int> selectedIndices;
    
    for (int i = 0; i < count; i++) {
        // ����ÿ����Ŀ��ѡ��״̬��ͨ�����鸴ѡ��״̬��
        DWORD itemData = SendMessageA(g_hRepoList, LB_GETITEMDATA, i, 0);
        if (itemData & 1) { // ����λΪ1��ʾѡ��
            selectedIndices.push_back(i);
        }
    }
    
    if (selectedIndices.empty()) {
        MessageBoxA(hwndDlg, "�����б���ѡ������һ���ֿ�", "��ʾ", MB_OK | MB_ICONWARNING);
        return;
    }

    // ��ȡĿ��·��
    char pathBuffer[MAX_PATH];
    GetWindowTextA(g_hPathEdit, pathBuffer, MAX_PATH);

    if (strlen(pathBuffer) == 0) {
        MessageBoxA(hwndDlg, "��ѡ�񱣴���Ŀ��·��", "��ʾ", MB_OK | MB_ICONWARNING);
        return;
    }

    // ���ð�ť��ֹ�ظ�����
    EnableWindow(g_hCloneButton, FALSE);
    EnableWindow(g_hFetchButton, FALSE);
    std::string status = "���ڿ�¡ " + std::to_string(selectedIndices.size()) + " ���ֿ�...";
    SetWindowTextA(g_hStatusLabel, status.c_str());

    // �����߳̿�¡�ֿ⣨����ѡ�е������б���
    CreateThread(NULL, 0, CloneRepositoryThread, new std::vector<int>(selectedIndices), 0, NULL);
}

DWORD WINAPI CloneRepositoryThread(LPVOID lpParam) {
    std::vector<int>* selectedIndices = (std::vector<int>*)lpParam;

    // ��ȡĿ��·��
    char pathBuffer[MAX_PATH];
    SendMessageA(g_hPathEdit, WM_GETTEXT, MAX_PATH, (LPARAM)pathBuffer);

    bool allSuccess = true;
    int clonedCount = 0;
    int totalToClone = selectedIndices->size();
    
    // ��¡ÿ��ѡ�еĲֿ�
    for (int index : *selectedIndices) {
        // ��ȡ�ֿ����ƣ��Ѿ���GBK���룩
        std::string repoName = g_repositories[index].name;
        std::string fullPath = std::string(pathBuffer) + "\\" + repoName;

        // ִ�п�¡������clone_url��UTF-8���룩
        bool success = GitHubAPI::cloneRepository(g_repositories[index].clone_url, fullPath);
        if (!success) {
            allSuccess = false;
        }
        clonedCount++;
        
        // ����״̬��Ϣ
        std::string status = "���ڿ�¡ " + std::to_string(clonedCount) + "/" + std::to_string(totalToClone) + " ���ֿ�...";
        SendMessageA(GetDlgItem(GetParent(g_hPathEdit), IDC_STATUS_LABEL), WM_SETTEXT, 0, (LPARAM)status.c_str());
    }

    // �����ڴ�
    delete selectedIndices;

    // ������Ϣ֪ͨ���̸߳���UI
    PostMessage(GetParent(g_hCloneButton), WM_USER + 2, (WPARAM)allSuccess, 0);

    return 0;
}

// UTF-8תGBK����
std::string Utf8ToGbk(const std::string& utf8Str) {
    if (utf8Str.empty()) return utf8Str;

    // �Ƚ�UTF-8ת��Ϊ���ַ�
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (wideCharLength <= 0) return utf8Str;

    std::vector<WCHAR> wideStr(wideCharLength);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideStr.data(), wideCharLength);

    // �ٽ����ַ�ת��ΪGBK
    int gbkLength = WideCharToMultiByte(936, 0, wideStr.data(), -1, NULL, 0, NULL, NULL);
    if (gbkLength <= 0) return utf8Str;

    std::vector<char> gbkStr(gbkLength);
    WideCharToMultiByte(936, 0, wideStr.data(), -1, gbkStr.data(), gbkLength, NULL, NULL);

    return std::string(gbkStr.data(), gbkLength - 1); // -1 to exclude null terminator
}/*
 * GitHub仓库克隆工具
 * Copyright (C) 2025 OpenSource Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */