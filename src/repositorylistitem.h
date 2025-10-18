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

#ifndef REPOSITORYLISTITEM_H
#define REPOSITORYLISTITEM_H

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include "github_api.h"

class RepositoryListItem : public QWidget
{
    Q_OBJECT

public:
    explicit RepositoryListItem(const Repository& repo, QWidget* parent = nullptr);
    
    QCheckBox* getCheckBox() const;
    Repository getRepository() const;

private:
    QCheckBox* checkBox;
    QLabel* nameLabel;
    QLabel* starsLabel;
    QLabel* descLabel;
    Repository repository;
};

#endif // REPOSITORYLISTITEM_H