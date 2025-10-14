#pragma once

#include "MainWindow.xaml.g.h"
#include "github_api.h"
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Foundation.h>

namespace winrt::GithubClone::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void CloneButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        GithubAPI m_githubAPI;
        winrt::Windows::Foundation::IAsyncAction CloneRepositoryAsync();
    };
}

namespace winrt::GithubClone::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}