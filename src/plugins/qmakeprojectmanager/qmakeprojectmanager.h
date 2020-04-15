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

#ifndef QMAKEPROJECTMANAGER_H
#define QMAKEPROJECTMANAGER_H

#include "qmakeprojectmanager_global.h"

#include <projectexplorer/iprojectmanager.h>
#include <projectexplorer/projectnodes.h>

namespace Core { class IEditor; }
namespace ExtensionSystem { class PluginManager; }

namespace ProjectExplorer {
class Project;
class Node;
class ToolChain;
}

namespace QmakeProjectManager {

namespace Internal {
class ProFileEditor;
class QmakeProjectManagerPlugin;
} // namespace Internal

class QmakeProject;

class QMAKEPROJECTMANAGER_EXPORT QmakeManager : public ProjectExplorer::IProjectManager
{
    Q_OBJECT

public:
    QmakeManager(Internal::QmakeProjectManagerPlugin *plugin);
    ~QmakeManager();

    void registerProject(QmakeProject *project);
    void unregisterProject(QmakeProject *project);
    void notifyChanged(const QString &name);

    virtual QString mimeType() const;
    ProjectExplorer::Project *openProject(const QString &fileName, QString *errorString);

    // Context information used in the slot implementations
    ProjectExplorer::Node *contextNode() const;
    void setContextNode(ProjectExplorer::Node *node);
    ProjectExplorer::Project *contextProject() const;
    void setContextProject(ProjectExplorer::Project *project);
    ProjectExplorer::FileNode *contextFile() const;
    void setContextFile(ProjectExplorer::FileNode *file);

    enum Action { BUILD, REBUILD, CLEAN };

public slots:
    void addLibrary();
    void addLibraryContextMenu();
    void runQMake();
    void runQMakeContextMenu();
    void buildSubDirContextMenu();
    void rebuildSubDirContextMenu();
    void cleanSubDirContextMenu();
    void buildFileContextMenu();
    void buildFile();

private:
    QList<QmakeProject *> m_projects;
    void handleSubDirContextMenu(Action action, bool isFileBuild);
    void handleSubDirContextMenu(QmakeManager::Action action, bool isFileBuild,
                                 ProjectExplorer::Project *contextProject,
                                 ProjectExplorer::Node *contextNode,
                                 ProjectExplorer::FileNode *contextFile);
    void addLibrary(const QString &fileName, Internal::ProFileEditor *editor = 0);
    void runQMake(ProjectExplorer::Project *p, ProjectExplorer::Node *node);

    Internal::QmakeProjectManagerPlugin *m_plugin;
    ProjectExplorer::Node *m_contextNode;
    ProjectExplorer::Project *m_contextProject;
    ProjectExplorer::FileNode *m_contextFile;
};

} // namespace QmakeProjectManager

#endif // QMAKEPROJECTMANAGER_H
