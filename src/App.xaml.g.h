#pragma once
#include "App.xaml.h"

namespace winrt::GithubClone::implementation
{
    template <typename D, typename ... I>
    struct App_baseWithProvider : public App_base<D, I...>
    {
        using composable = D;
    protected:
        App_baseWithProvider()
        {
            impl::call_factory<App, IAppStatics>([&](IAppStatics const& f) { f.CreateInstance(*this); });
        }
    };
    template <typename D, typename ... I>
    using AppT2 = App_baseWithProvider<D, I...>;
}
namespace winrt::GithubClone
{
    struct App : AppT2<App, implementation::App>
    {
        App() = default;
        using AppT2<App, implementation::App>::App;
    };
}