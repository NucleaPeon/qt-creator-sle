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

#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include "projectexplorer_export.h"
#include "buildstep.h"

#include <QObject>
#include <QStringList>

namespace ProjectExplorer {

class Task;
class Project;

class PROJECTEXPLORER_EXPORT BuildManager : public QObject
{
    Q_OBJECT

public:
    explicit BuildManager(QObject *parent, QAction *cancelBuildAction);
    ~BuildManager();
    static QObject *instance();

    static void extensionsInitialized();

    static bool isBuilding();
    static bool tasksAvailable();

    static bool buildLists(QList<BuildStepList *> bsls, const QStringList &stepListNames,
                    const QStringList &preambelMessage = QStringList());
    static bool buildList(BuildStepList *bsl, const QString &stepListName);

    static bool isBuilding(Project *p);
    static bool isBuilding(Target *t);
    static bool isBuilding(ProjectConfiguration *p);
    static bool isBuilding(BuildStep *step);

    // Append any build step to the list of build steps (currently only used to add the QMakeStep)
    static void appendStep(BuildStep *step, const QString &name);

    static int getErrorTaskCount();

public slots:
    static void cancel();
    // Shows without focus
    static void showTaskWindow();
    static void toggleTaskWindow();
    static void toggleOutputWindow();
    static void aboutToRemoveProject(ProjectExplorer::Project *p);

signals:
    void buildStateChanged(ProjectExplorer::Project *pro);
    void buildQueueFinished(bool success);
    void tasksChanged();
    void taskAdded(const ProjectExplorer::Task &task);
    void tasksCleared();

private slots:
    static void addToTaskWindow(const ProjectExplorer::Task &task);
    static void addToOutputWindow(const QString &string, ProjectExplorer::BuildStep::OutputFormat,
        ProjectExplorer::BuildStep::OutputNewlineSetting = BuildStep::DoAppendNewline);

    static void buildStepFinishedAsync();
    static void nextBuildQueue();
    static void progressChanged();
    static void progressTextChanged();
    static void emitCancelMessage();
    static void showBuildResults();
    static void updateTaskCount();
    static void finish();

private:
    static void startBuildQueue(const QStringList &preambleMessage = QStringList());
    static void nextStep();
    static void clearBuildQueue();
    static bool buildQueueAppend(QList<BuildStep *> steps, QStringList names);
    static void incrementActiveBuildSteps(BuildStep *bs);
    static void decrementActiveBuildSteps(BuildStep *bs);
    static void disconnectOutput(BuildStep *bs);
};

} // namespace ProjectExplorer

#endif // BUILDMANAGER_H
