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

#ifndef ANDROIDSIGNALOPERATION_H
#define ANDROIDSIGNALOPERATION_H

#include <projectexplorer/devicesupport/idevice.h>

#include <QObject>
#include <QProcess>
#include <QTimer>

namespace Android {
namespace Internal {

class AndroidSignalOperation : public ProjectExplorer::DeviceProcessSignalOperation
{
    Q_OBJECT
public:
    ~AndroidSignalOperation() {}
    void killProcess(int pid);
    void killProcess(const QString &filePath);
    void interruptProcess(int pid);
    void interruptProcess(const QString &filePath);

protected:
    explicit AndroidSignalOperation();

private slots:
    void adbFindRunAsFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void adbKillFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleTimeout();

private:
    void signalOperationViaADB(int pid, int signal);

    QString m_adbPath;
    QProcess *m_adbProcess;
    QTimer *m_timeout;

    enum State {
        Idle,
        RunAs,
        Kill
    } m_state;

    int m_pid;
    int m_signal;

    friend class AndroidDevice;
};

} // namespace Internal
} // namespace Android

#endif // ANDROIDSIGNALOPERATION_H
