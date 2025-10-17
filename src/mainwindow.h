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