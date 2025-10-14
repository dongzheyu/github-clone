#pragma once

// 注意：这些头文件路径可能需要根据实际安装位置调整
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>

namespace winrt::GithubClone::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void CloneButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        // UI elements
        Microsoft::UI::Xaml::Controls::TextBox RepoUrlTextBox() { 
            return FindName(L"RepoUrlTextBox").as<Microsoft::UI::Xaml::Controls::TextBox>(); 
        }
        
        Microsoft::UI::Xaml::Controls::Button CloneButton() { 
            return FindName(L"CloneButton").as<Microsoft::UI::Xaml::Controls::Button>(); 
        }
        
        Microsoft::UI::Xaml::Controls::ProgressBar CloneProgress() { 
            return FindName(L"CloneProgress").as<Microsoft::UI::Xaml::Controls::ProgressBar>(); 
        }
        
        Microsoft::UI::Xaml::Controls::TextBlock StatusText() { 
            return FindName(L"StatusText").as<Microsoft::UI::Xaml::Controls::TextBlock>(); 
        }
        
        winrt::Windows::Foundation::IAsyncAction CloneRepositoryAsync();
    };
}