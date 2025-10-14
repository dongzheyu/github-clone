#pragma once
#include "App.xaml.g.h"
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>

namespace winrt::GithubClone::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(winrt::Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);

    private:
        winrt::Microsoft::UI::Xaml::Window window{ nullptr };
    };
}