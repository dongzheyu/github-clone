/*
 * GitHub仓库克隆工具
 * Copyright (C) 2025 OpenSource Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "repositorylistitem.h"
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QMetaType>
#include <QListWidgetItem>
#include <QScrollArea>
#include <thread>
#include "github_api.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , selectedIndex(-1)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI() {
    // 设置主窗口属性
    setWindowTitle("GitHub仓库克隆工具");
    resize(800, 600);
    
    // 创建中央部件和主布局
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 用户名输入区域
    QHBoxLayout* usernameLayout = new QHBoxLayout;
    QLabel* usernameLabel = new QLabel("GitHub用户名:");
    usernameEdit = new QLineEdit;
    fetchButton = new QPushButton("获取仓库");
    
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(usernameEdit);
    usernameLayout->addWidget(fetchButton);
    
    // 仓库列表（使用滚动区域）
    QLabel* repoListLabel = new QLabel("仓库列表:");
    repoList = new QListWidget;
    repoList->setSelectionMode(QAbstractItemView::NoSelection); // 禁用默认选择模式
    
    // 路径选择区域
    QHBoxLayout* pathLayout = new QHBoxLayout;
    QLabel* pathLabel = new QLabel("克隆到:");
    pathEdit = new QLineEdit;
    browseButton = new QPushButton("浏览...");
    
    pathLayout->addWidget(pathLabel);
    pathLayout->addWidget(pathEdit);
    pathLayout->addWidget(browseButton);
    
    // 克隆按钮
    cloneButton = new QPushButton("克隆选中仓库");
    cloneButton->setEnabled(false);
    
    // 状态栏
    statusLabel = new QLabel("就绪");
    progressBar = new QProgressBar;
    progressBar->setVisible(false);
    
    // 添加所有控件到主布局
    mainLayout->addLayout(usernameLayout);
    mainLayout->addWidget(repoListLabel);
    mainLayout->addWidget(repoList);
    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(cloneButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(progressBar);
    
    // 连接信号和槽
    connect(fetchButton, &QPushButton::clicked, this, &MainWindow::onFetchButtonClicked);
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseButtonClicked);
    connect(cloneButton, &QPushButton::clicked, this, &MainWindow::onCloneButtonClicked);
    connect(pathEdit, &QLineEdit::textChanged, [this]() {
        // 检查是否有选中的仓库
        bool hasChecked = false;
        for (int i = 0; i < repoList->count(); ++i) {
            QListWidgetItem* item = repoList->item(i);
            QWidget* widget = repoList->itemWidget(item);
            if (RepositoryListItem* repoItem = qobject_cast<RepositoryListItem*>(widget)) {
                if (repoItem->getCheckBox()->isChecked()) {
                    hasChecked = true;
                    break;
                }
            }
        }
        cloneButton->setEnabled(hasChecked && !pathEdit->text().isEmpty());
    });
}

void MainWindow::onFetchButtonClicked() {
    QString username = usernameEdit->text();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入GitHub用户名");
        return;
    }
    
    fetchButton->setEnabled(false);
    statusLabel->setText("正在获取仓库列表...");
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // 设置为忙碌状态
    
    // 在新线程中获取仓库列表
    std::thread([this, username]() {
        QString qUsername = username;
        std::string stdUsername = qUsername.toStdString();
        
        // 获取仓库列表
        std::vector<Repository> repos = GitHubAPI::getUserRepositories(stdUsername);
        
        // 在主线程中更新UI
        QMetaObject::invokeMethod(this, [this, repos]() {
            repositories = repos;
            updateRepoList();
            fetchButton->setEnabled(true);
            progressBar->setVisible(false);
            
            if (repositories.empty()) {
                statusLabel->setText("未找到仓库");
            } else {
                statusLabel->setText(QString("找到 %1 个仓库").arg(repositories.size()));
            }
        });
    }).detach();
}

void MainWindow::updateRepoList() {
    repoList->clear();
    
    for (const auto& repo : repositories) {
        QListWidgetItem* item = new QListWidgetItem();
        RepositoryListItem* repoItem = new RepositoryListItem(repo);
        item->setSizeHint(repoItem->sizeHint());
        repoList->addItem(item);
        repoList->setItemWidget(item, repoItem);
        
        // 连接复选框状态变化信号
        connect(repoItem->getCheckBox(), &QCheckBox::stateChanged, [this]() {
            // 检查是否有选中的仓库
            bool hasChecked = false;
            for (int i = 0; i < repoList->count(); ++i) {
                QListWidgetItem* item = repoList->item(i);
                QWidget* widget = repoList->itemWidget(item);
                if (RepositoryListItem* repoItem = qobject_cast<RepositoryListItem*>(widget)) {
                    if (repoItem->getCheckBox()->isChecked()) {
                        hasChecked = true;
                        break;
                    }
                }
            }
            cloneButton->setEnabled(hasChecked && !pathEdit->text().isEmpty());
        });
    }
    
    cloneButton->setEnabled(false);
}

void MainWindow::onBrowseButtonClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, "选择克隆目录", 
                                                    QDir::homePath());
    if (!dir.isEmpty()) {
        pathEdit->setText(dir);
    }
}

void MainWindow::onCloneButtonClicked() {
    // 收集所有选中的仓库
    QList<std::pair<int, Repository>> selectedRepos;
    for (int i = 0; i < repoList->count(); ++i) {
        QListWidgetItem* item = repoList->item(i);
        QWidget* widget = repoList->itemWidget(item);
        if (RepositoryListItem* repoItem = qobject_cast<RepositoryListItem*>(widget)) {
            if (repoItem->getCheckBox()->isChecked()) {
                selectedRepos.append({i, repositories[i]});
            }
        }
    }
    
    if (selectedRepos.isEmpty()) {
        QMessageBox::warning(this, "提示", "请从列表中选择至少一个仓库");
        return;
    }
    
    QString path = pathEdit->text();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择目标路径");
        return;
    }
    
    cloneButton->setEnabled(false);
    fetchButton->setEnabled(false);
    statusLabel->setText(QString("正在克隆 %1 个仓库...").arg(selectedRepos.size()));
    progressBar->setVisible(true);
    progressBar->setRange(0, selectedRepos.size());
    progressBar->setValue(0);
    
    // 在新线程中克隆仓库
    std::thread([this, path, selectedRepos]() {
        QString qPath = path;
        int clonedCount = 0;
        int totalCount = selectedRepos.size();
        bool hasError = false;
        
        for (const auto& pair : selectedRepos) {
            int index = pair.first;
            const Repository& repo = pair.second;
            std::string fullPath = qPath.toStdString() + "/" + repo.name;
            
            bool success = GitHubAPI::cloneRepository(repo.clone_url, fullPath);
            if (!success) {
                hasError = true;
            }
            
            clonedCount++;
            
            // 在主线程中更新进度
            QMetaObject::invokeMethod(this, [this, clonedCount, totalCount]() {
                progressBar->setValue(clonedCount);
            });
        }
        
        // 在主线程中更新UI
        QMetaObject::invokeMethod(this, [this, clonedCount, totalCount, hasError]() {
            cloneButton->setEnabled(true);
            fetchButton->setEnabled(true);
            progressBar->setVisible(false);
            
            if (hasError) {
                statusLabel->setText(QString("部分仓库克隆失败 (%1/%2)").arg(clonedCount).arg(totalCount));
                QMessageBox::warning(this, "完成", QString("部分仓库克隆失败 (%1/%2)").arg(clonedCount).arg(totalCount));
            } else {
                statusLabel->setText(QString("成功克隆 %1 个仓库").arg(totalCount));
                QMessageBox::information(this, "成功", QString("成功克隆 %1 个仓库").arg(totalCount));
            }
        });
    }).detach();
}

std::string MainWindow::utf8ToGbk(const std::string& utf8Str) {
    // Qt版本不需要手动转换编码，注释掉这部分代码
    return utf8Str;
    
    /*
    if (utf8Str.empty()) return utf8Str;

    // 先将UTF-8转换为宽字符
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (wideCharLength <= 0) return utf8Str;

    std::vector<WCHAR> wideStr(wideCharLength);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideStr.data(), wideCharLength);

    // 再将宽字符转换为GBK
    int gbkLength = WideCharToMultiByte(936, 0, wideStr.data(), -1, NULL, 0, NULL, NULL);
    if (gbkLength <= 0) return utf8Str;

    std::vector<char> gbkStr(gbkLength);
    WideCharToMultiByte(936, 0, wideStr.data(), -1, gbkStr.data(), gbkLength, NULL, NULL);

    return std::string(gbkStr.data(), gbkLength - 1); // -1 to exclude null terminator
    */
}