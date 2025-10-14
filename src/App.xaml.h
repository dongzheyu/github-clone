#pragma once

#include "App.xaml.g.h"
#include <winrt/Microsoft.UI.Xaml.h>

namespace winrt::GithubClone::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
    };
}

namespace winrt::GithubClone::factory_implementation
{
    struct App : AppT<App, implementation::App>
    {
    };
}