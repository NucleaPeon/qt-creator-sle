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

#ifndef WINRTPACKAGEDEPLOYMENTSTEP_H
#define WINRTPACKAGEDEPLOYMENTSTEP_H

#include <projectexplorer/abstractprocessstep.h>

namespace WinRt {
namespace Internal {

class WinRtPackageDeploymentStep : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT
public:
    explicit WinRtPackageDeploymentStep(ProjectExplorer::BuildStepList *bsl);
    bool init();
    bool processSucceeded(int exitCode, QProcess::ExitStatus status);
    void stdOutput(const QString &line);
    ProjectExplorer::BuildStepConfigWidget *createConfigWidget();

    void setWinDeployQtArguments(const QString &args);
    QString winDeployQtArguments() const;
    QString defaultWinDeployQtArguments() const;

    void raiseError(const QString &errorMessage);

    bool fromMap(const QVariantMap &map);
    QVariantMap toMap() const;

private:
    bool parseIconsAndExecutableFromManifest(QString manifestFileName, QStringList *items, QString *executable);

    QString m_args;
    QString m_executablePathInManifest;
    QString m_mappingFileContent;
    QString m_manifestFileName;
    bool m_createMappingFile;
};

} // namespace Internal
} // namespace WinRt

#endif // WINRTPACKAGEDEPLOYMENTSTEP_H
