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