#include "repositorylistitem.h"

RepositoryListItem::RepositoryListItem(const Repository& repo, QWidget* parent)
    : QWidget(parent), repository(repo)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 2, 5, 2);
    
    checkBox = new QCheckBox(this);
    nameLabel = new QLabel(QString::fromStdString(repo.name), this);
    starsLabel = new QLabel(QString("(%1 stars)").arg(repo.stars), this);
    descLabel = new QLabel(QString::fromStdString(repo.description), this);
    
    // 设置标签属性
    nameLabel->setStyleSheet("font-weight: bold;");
    starsLabel->setStyleSheet("color: gray;");
    
    layout->addWidget(checkBox);
    layout->addWidget(nameLabel);
    layout->addWidget(starsLabel);
    layout->addWidget(descLabel, 1); // stretch factor to push other items left
    
    setLayout(layout);
}

QCheckBox* RepositoryListItem::getCheckBox() const {
    return checkBox;
}

Repository RepositoryListItem::getRepository() const {
    return repository;
}