#include "utils.h"
#include "github_api.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <comdef.h>
#define access _access
#define F_OK 0
#else
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#endif

bool Utils::isGitInstalled() {
#ifdef _WIN32
    // Windows下检查Git
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\GitForWindows", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    
    // 检查PATH中是否有git
    const char* pathEnv = getenv("PATH");
    if (pathEnv && strstr(pathEnv, "Git")) {
        return true;
    }
    
    // 尝试执行git命令
    return system("git --version >nul 2>&1") == 0;
#else
    // Unix-like系统检查git
    return system("which git > /dev/null 2>&1") == 0 || system("git --version > /dev/null 2>&1") == 0;
#endif
}

int Utils::selectRepositoryCLI(const std::vector<Repository>& repos) {
    if (repos.empty()) {
        std::cout << "The user has no public repositories.\n";
        return -1;
    }
    
    std::cout << "\nFound " << repos.size() << " repositories:\n";
    for (size_t i = 0; i < repos.size(); ++i) {
        std::cout << i + 1 << ". " << repos[i].name << " (⭐" << repos[i].stars << ")\n";
        if (!repos[i].description.empty() && repos[i].description != "null") {
            std::cout << "   Description: " << repos[i].description << "\n";
        }
        std::cout << "   URL: " << repos[i].clone_url << "\n\n";
    }
    
    int choice;
    do {
        std::cout << "Please select a repository to clone (1-" << repos.size() << ", 0 to cancel): ";
        std::cin >> choice;
        
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            choice = -1;
        }
        
        if (choice == 0) {
            return -1; // User cancelled
        }
        
        if (choice < 0 || choice > (int)repos.size()) {
            std::cout << "Invalid selection, please try again.\n";
        }
    } while (choice < 1 || choice > (int)repos.size());
    
    return choice - 1; // Return 0-based index
}

std::string Utils::getInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n'); // Clear input buffer
    std::getline(std::cin, input);
    return input;
}

std::string Utils::selectDirectory(const std::string& prompt) {
#ifdef _WIN32
    return selectDirectoryWin32();
#else
    return getInput(prompt);
#endif
}

#ifdef _WIN32
std::string Utils::selectDirectoryWin32() {
    CoInitialize(NULL);
    
    IFileOpenDialog *pFileOpen;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    
    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        pFileOpen->GetOptions(&dwOptions);
        pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
        
        hr = pFileOpen->Show(NULL);
        
        if (SUCCEEDED(hr)) {
            IShellItem *pItem;
            hr = pFileOpen->GetResult(&pItem);
            
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                
                if (SUCCEEDED(hr)) {
                    // Convert wide characters to multibyte characters
                    int size_needed = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, NULL, 0, NULL, NULL);
                    std::string result(size_needed - 1, 0);
                    WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &result[0], size_needed, NULL, NULL);
                    
                    CoTaskMemFree(pszFilePath);
                    pItem->Release();
                    pFileOpen->Release();
                    CoUninitialize();
                    return result;
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
    
    CoUninitialize();
    return ""; // If dialog is cancelled or error occurs
}
#endif