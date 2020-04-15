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

#ifndef QTC_LINUXDEVICEPROCESS_H
#define QTC_LINUXDEVICEPROCESS_H

#include "remotelinux_export.h"

#include <projectexplorer/devicesupport/sshdeviceprocess.h>

#include <QStringList>

namespace RemoteLinux {

class REMOTELINUX_EXPORT LinuxDeviceProcess : public ProjectExplorer::SshDeviceProcess
{
    Q_OBJECT
public:
    explicit LinuxDeviceProcess(const QSharedPointer<const ProjectExplorer::IDevice> &device,
                                QObject *parent = 0);

    // Files to source before executing the command (if they exist). Overrides the default.
    void setRcFilesToSource(const QStringList &filePaths);

    void setWorkingDirectory(const QString &directory);

private:
    QString fullCommandLine() const;

    QStringList rcFilesToSource() const;

    QStringList m_rcFilesToSource;
    QString m_workingDir;
};

} // namespace RemoteLinux

#endif // Include guard.
