#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#include <winrt/Windows.ApplicationModel.Activation.h>

namespace winrt::GithubClone::implementation
{
    App::App()
    {
        InitializeComponent();
    }

    void App::OnLaunched(winrt::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&)
    {
        window = make<MainWindow>();
        window.Activate();
    }
}