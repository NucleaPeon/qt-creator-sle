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

#ifndef QTC_DESKTOPDEVICEPROCESS_H
#define QTC_DESKTOPDEVICEPROCESS_H

#include "deviceprocess.h"

namespace ProjectExplorer {
namespace Internal {
class ProcessHelper;

class DesktopDeviceProcess : public DeviceProcess
{
    Q_OBJECT
public:
    DesktopDeviceProcess(const QSharedPointer<const IDevice> &device, QObject *parent = 0);

    void start(const QString &executable, const QStringList &arguments);
    void interrupt();
    void terminate();
    void kill();

    QProcess::ProcessState state() const;
    QProcess::ExitStatus exitStatus() const;
    int exitCode() const;
    QString errorString() const;

    Utils::Environment environment() const;
    void setEnvironment(const Utils::Environment &env);

    void setWorkingDirectory(const QString &directory);

    QByteArray readAllStandardOutput();
    QByteArray readAllStandardError();

    qint64 write(const QByteArray &data);

private:
    QProcess * const m_process;
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // Include guard
