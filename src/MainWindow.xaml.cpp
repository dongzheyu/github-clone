#include "MainWindow.xaml.h"
#include "github_api.h"
#include "utils.h"
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <thread>

namespace winrt::GithubClone::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        
        // 初始化UI状态
        CloneButton().IsEnabled(false);
    }

    void MainWindow::FetchButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        auto username = to_string(UsernameTextBox().Text());
        if (username.empty())
        {
            StatusTextBlock().Text(L"请输入GitHub用户名");
            return;
        }

        // 清空之前的仓库列表
        RepoListBox().Items().Clear();
        
        // 更新UI状态
        FetchButton().IsEnabled(false);
        StatusTextBlock().Text(L"正在获取仓库列表...");
        
        // 在新线程中获取仓库信息
        std::thread([this, username]()
        {
            // 获取仓库列表 - 从API获取的是UTF-8编码
            std::vector<Repository> repos = GitHubAPI::getUserRepositories(username);
            
            // 切换回UI线程更新界面
            Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [this, repos]()
                {
                    UpdateRepoList(repos);
                });
        }).detach();
    }

    void MainWindow::UpdateRepoList(std::vector<Repository> const& repos)
    {
        // 启用获取按钮
        FetchButton().IsEnabled(true);

        if (repos.empty())
        {
            StatusTextBlock().Text(L"未找到该用户的所有仓库");
            return;
        }

        // 填充仓库列表
        int validRepoCount = 0;
        for (const auto& repo : repos)
        {
            // 过滤无效仓库（没有名称和URL的）
            if (repo.name.empty() || repo.clone_url.empty())
            {
                continue;
            }

            // 使用UTF-16编码的字符串显示
            std::string itemText = repo.name + " (" + std::to_string(repo.stars) + ")";

            // 添加描述信息
            if (!repo.description.empty())
            {
                itemText += " - " + repo.description;
            }
            else
            {
                itemText += " - 无描述";
            }

            RepoListBox().Items().Append(winrt::box_value(to_hstring(itemText)));
            validRepoCount++;
        }

        if (validRepoCount == 0)
        {
            StatusTextBlock().Text(L"未找到有效仓库");
            return;
        }

        std::string statusText = "找到 " + std::to_string(validRepoCount) + " 个仓库";
        StatusTextBlock().Text(to_hstring(statusText));
        CloneButton().IsEnabled(true);
    }

    void MainWindow::BrowseButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        // 在新线程中打开文件选择器
        std::thread([this]()
        {
            Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [this]()
                {
                    auto picker = Windows::Storage::Pickers::FolderPicker();
                    picker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::Desktop);
                    picker.FileTypeFilter().Append(L"*");
                    
                    auto asyncOp = picker.PickSingleFolderAsync();
                    asyncOp.Completed([this](auto&& asyncOp, auto&& status)
                        {
                            if (status == Windows::Foundation::AsyncStatus::Completed)
                            {
                                auto folder = asyncOp.GetResults();
                                if (folder)
                                {
                                    PathTextBox().Text(folder.Path());
                                }
                            }
                        });
                });
        }).detach();
    }

    void MainWindow::CloneButton_Click(IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        // 检查是否选择了仓库
        if (RepoListBox().SelectedIndex() == -1)
        {
            StatusTextBlock().Text(L"请在列表中选择一个仓库");
            return;
        }

        auto path = to_string(PathTextBox().Text());
        if (path.empty())
        {
            StatusTextBlock().Text(L"请选择或输入目标路径");
            return;
        }

        // 禁用按钮防止重复点击
        CloneButton().IsEnabled(false);
        FetchButton().IsEnabled(false);
        StatusTextBlock().Text(L"正在克隆仓库...");

        // 获取选中的仓库索引
        int selectedIndex = static_cast<int>(RepoListBox().SelectedIndex());

        // 在新线程中克隆仓库
        std::thread([this, path, selectedIndex]()
        {
            // 获取仓库列表
            // 注意：这里需要重新获取仓库列表或者以其他方式传递仓库信息
            auto username = to_string(UsernameTextBox().Text());
            std::vector<Repository> repos = GitHubAPI::getUserRepositories(username);
            
            // 构造完整路径
            std::string repoName = repos[selectedIndex].name;
            std::string fullPath = path + "\\" + repoName;

            // 执行克隆操作
            bool success = GitHubAPI::cloneRepository(repos[selectedIndex].clone_url, fullPath);

            // 切换回UI线程更新界面
            Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [this, success]()
                {
                    if (success)
                    {
                        StatusTextBlock().Text(L"仓库克隆成功！");
                        Windows::UI::Popups::MessageDialog(L"仓库克隆成功！", L"成功").ShowAsync();
                    }
                    else
                    {
                        StatusTextBlock().Text(L"仓库克隆失败！");
                        Windows::UI::Popups::MessageDialog(L"仓库克隆失败！", L"错误").ShowAsync();
                    }
                    
                    // 重新启用按钮
                    FetchButton().IsEnabled(true);
                    CloneButton().IsEnabled(true);
                });
        }).detach();
    }
}