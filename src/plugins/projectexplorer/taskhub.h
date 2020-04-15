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

#ifndef TASKHUB_H
#define TASKHUB_H

#include "task.h"

#include <QObject>
#include <QIcon>

namespace ProjectExplorer {

class ProjectExplorerPlugin;

class PROJECTEXPLORER_EXPORT TaskHub : public QObject
{
    Q_OBJECT
public:
    static QObject *instance();

    // Convenience overload
    static void addTask(Task::TaskType type, const QString &description,
                        Core::Id category,
                        const Utils::FileName &file = Utils::FileName(),
                        int line = -1);

public slots:
    static void addTask(ProjectExplorer::Task task);
    static void clearTasks(Core::Id categoryId = Core::Id());
    static void removeTask(const ProjectExplorer::Task &task);

public:
    static void addCategory(Core::Id categoryId, const QString &displayName, bool visible = true);
    static void updateTaskFileName(unsigned int id, const QString &fileName);
    static void updateTaskLineNumber(unsigned int id, int line);
    static void taskMarkClicked(unsigned int id);
    static void showTaskInEditor(unsigned int id);
    static void setCategoryVisibility(const Core::Id &categoryId, bool visible);

    static void requestPopup();

signals:
    void categoryAdded(Core::Id categoryId, const QString &displayName, bool visible);
    void taskAdded(const ProjectExplorer::Task &task);
    void taskRemoved(const ProjectExplorer::Task &task);
    void tasksCleared(Core::Id categoryId);
    void taskFileNameUpdated(unsigned int id, const QString &fileName);
    void taskLineNumberUpdated(unsigned int id, int line);
    void categoryVisibilityChanged(const Core::Id &categoryId, bool visible);
    void popupRequested(int);
    void showTask(unsigned int id);
    void openTask(unsigned int id);

private:
    TaskHub();
    ~TaskHub();

    const QIcon m_errorIcon;
    const QIcon m_warningIcon;

    friend class ProjectExplorerPlugin;
};

} // namespace ProjectExplorer
#endif // TASKHUB_H
