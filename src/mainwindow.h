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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <vector>
#include "github_api.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class RepositoryListItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFetchButtonClicked();
    void onBrowseButtonClicked();
    void onCloneButtonClicked();
    void updateRepoList();

private:
    // UI组件
    QLineEdit* usernameEdit;
    QPushButton* fetchButton;
    QListWidget* repoList;
    QLineEdit* pathEdit;
    QPushButton* browseButton;
    QPushButton* cloneButton;
    QLabel* statusLabel;
    QProgressBar* progressBar;
    
    // 数据
    std::vector<Repository> repositories;
    int selectedIndex;
    
    // 方法
    void setupUI();
    std::string utf8ToGbk(const std::string& utf8Str);
};

#endif // MAINWINDOW_H