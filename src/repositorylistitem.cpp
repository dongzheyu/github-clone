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