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

#ifndef QBSPROJECTMANAGER_H
#define QBSPROJECTMANAGER_H

#include "qbsprojectmanager_global.h"

#include <projectexplorer/iprojectmanager.h>

namespace qbs {
class Settings;
class Preferences;
} // namespace qbs

#include <QString>
#include <QVariantMap>

namespace ProjectExplorer {
class Kit;
class Project;
class ProjectExplorerPlugin;
} // namespace ProjectExplorer

namespace QbsProjectManager {

namespace Internal {
class QbsLogSink;
class QbsProject;
class QbsProjectManagerPlugin;
} // namespace Internal

class DefaultPropertyProvider;

class QbsManager : public ProjectExplorer::IProjectManager
{
    Q_OBJECT

public:
    QbsManager(Internal::QbsProjectManagerPlugin *plugin);
    ~QbsManager();

    QString mimeType() const;
    ProjectExplorer::Project *openProject(const QString &fileName, QString *errorString);

    // QBS profiles management:
    QString profileForKit(const ProjectExplorer::Kit *k) const;
    void setProfileForKit(const QString &name, const ProjectExplorer::Kit *k);

    static qbs::Settings *settings();
    Internal::QbsLogSink *logSink() { return m_logSink; }

private slots:
    void pushKitsToQbs();

private:
    void addProfile(const QString &name, const QVariantMap &data);
    void removeCreatorProfiles();
    void addQtProfileFromKit(const QString &profileName, const ProjectExplorer::Kit *k);
    void addProfileFromKit(const ProjectExplorer::Kit *k);

    Internal::QbsProjectManagerPlugin *m_plugin;
    Internal::QbsLogSink *m_logSink;
    static qbs::Settings *m_settings;

    DefaultPropertyProvider *m_defaultPropertyProvider;
};

} // namespace QbsProjectManager

#endif // QBSPROJECTMANAGER_H
