#pragma once

#include "MainWindow.xaml.g.h"
#include "github_api.h"
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <string>
#include <vector>

namespace winrt::GithubClone::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void FetchButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void BrowseButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);
        void CloneButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&);

    private:
        void UpdateRepoList(std::vector<Repository> const& repos);
        std::wstring to_hstring(const std::string& s)
        {
            int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
            std::wstring result(len, 0);
            MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &result[0], len);
            return result;
        }
        
        std::string to_string(const std::wstring& s)
        {
            int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string result(len, 0);
            WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &result[0], len, nullptr, nullptr);
            return result;
        }
    };
}