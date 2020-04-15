/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef DEPENDENCIESPANEL_H
#define DEPENDENCIESPANEL_H

#include "iprojectproperties.h"

#include <QAbstractListModel>

#include <QTreeView>

namespace Utils { class DetailsWidget; }

namespace ProjectExplorer {

class Project;

namespace Internal {

class DependenciesPanelFactory : public IProjectPanelFactory
{
public:
    DependenciesPanelFactory() {}

    QString id() const;
    QString displayName() const;
    int priority() const;
    bool supports(Project *project);
    PropertiesPanel *createPanel(Project *project);
};


//
// DependenciesModel
//

class DependenciesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DependenciesModel(Project *project, QObject *parent = 0);

    int rowCount(const QModelIndex &index) const;
    int columnCount(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

public slots:
    void resetModel();

private:
    Project *m_project;
    QList<Project *> m_projects;
};

class DependenciesView : public QTreeView
{
    Q_OBJECT

public:
    DependenciesView(QWidget *parent);

    QSize sizeHint() const;
    void setModel(QAbstractItemModel *model);

private slots:
    void updateSizeHint();

private:
    QSize m_sizeHint;
};

class DependenciesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DependenciesWidget(Project *project, QWidget *parent = 0);

private:
    Project *m_project;
    DependenciesModel *m_model;
    Utils::DetailsWidget *m_detailsContainer;
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // DEPENDENCIESPANEL_H
