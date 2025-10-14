#include "MainWindow.xaml.h"
#include "github_api.h"
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <sstream>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Windows::Storage;

namespace winrt::GithubClone::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        
        // Register button click event
        CloneButton().Click({ this, &MainWindow::CloneButton_Click });
    }

    void MainWindow::CloneButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        CloneRepositoryAsync();
    }

    Windows::Foundation::IAsyncAction MainWindow::CloneRepositoryAsync()
    {
        // Get the entered repository URL
        hstring repoUrl = RepoUrlTextBox().Text();
        
        if (repoUrl.empty())
        {
            StatusText().Text(L"Please enter a repository URL");
            co_return;
        }

        // Disable button and show progress bar
        CloneButton().IsEnabled(false);
        CloneProgress().IsIndeterminate(true);
        CloneProgress().Visibility(Visibility::Visible);
        StatusText().Text(L"Cloning...");

        try
        {
            // In a real application, cloning operation would be performed here
            // For demonstration purposes, we just show a message
            co_await winrt::Windows::System::Threading::ThreadPool::RunAsync([](auto&)
            {
                // Simulate cloning operation
                Sleep(3000);
            });
            
            StatusText().Text(L"Clone completed!");
        }
        catch (...)
        {
            StatusText().Text(L"Clone failed!");
        }

        // Re-enable button and hide progress bar
        CloneProgress().Visibility(Visibility::Collapsed);
        CloneButton().IsEnabled(true);
    }
}