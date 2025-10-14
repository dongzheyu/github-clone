#pragma once
#include "MainWindow.xaml.h"

namespace winrt::GithubClone::implementation
{
    template <typename D, typename ... I>
    struct MainWindow_baseWithProvider : public MainWindow_base<D, I...>
    {
        using composable = D;
    protected:
        MainWindow_baseWithProvider()
        {
            impl::call_factory<MainWindow, IMainWindowStatics>([&](IMainWindowStatics const& f) { f.CreateInstance(*this); });
        }
    };
    template <typename D, typename ... I>
    using MainWindowT2 = MainWindow_baseWithProvider<D, I...>;
}
namespace winrt::GithubClone
{
    struct MainWindow : MainWindowT2<MainWindow, implementation::MainWindow>
    {
        MainWindow() = default;
        using MainWindowT2<MainWindow, implementation::MainWindow>::MainWindow;
    };
}