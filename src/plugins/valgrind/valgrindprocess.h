/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
** Author: Milian Wolff, KDAB (milian.wolff@kdab.com)
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

#ifndef VALGRINDPROCESS_H
#define VALGRINDPROCESS_H

#include <utils/qtcprocess.h>
#include <ssh/sshremoteprocess.h>
#include <ssh/sshconnection.h>
#include <utils/outputformat.h>

namespace Valgrind {

struct Remote {
    QSsh::SshConnectionParameters m_params;
    QSsh::SshConnection *m_connection;
    QSsh::SshRemoteProcess::Ptr m_process;
    QString m_workingDir;
    QString m_valgrindExe;
    QString m_debuggee;
    QString m_errorString;
    QProcess::ProcessError m_error;
    QSsh::SshRemoteProcess::Ptr m_findPID;
};

/**
 * Process for supplying local and remote valgrind runs
 */
class ValgrindProcess : public QObject
{
    Q_OBJECT

public:
    ValgrindProcess(bool isLocal, const QSsh::SshConnectionParameters &sshParams,
                    QSsh::SshConnection *connection = 0, QObject *parent = 0);

    bool isRunning() const;

    void run(const QString &valgrindExecutable, const QStringList &valgrindArguments,
             const QString &debuggeeExecutable, const QString &debuggeeArguments);
    void close();

    QString errorString() const;
    QProcess::ProcessError error() const;

    void setProcessChannelMode(QProcess::ProcessChannelMode mode);
    void setWorkingDirectory(const QString &path);
    QString workingDirectory() const;
    void setEnvironment(const Utils::Environment &environment);

    qint64 pid() const;
    QSsh::SshConnection *connection() const;
    bool isLocal() const { return m_isLocal; }

signals:
    void started();
    void finished(int, QProcess::ExitStatus);
    void error(QProcess::ProcessError);
    void processOutput(const QByteArray &, Utils::OutputFormat format);

private slots:
    void handleReadyReadStandardError();
    void handleReadyReadStandardOutput();
    void handleError(QSsh::SshError);

    void closed(int);
    void connected();
    void processStarted();
    void findPIDOutputReceived();

private:
    Utils::QtcProcess m_localProcess;
    qint64 m_pid;

    Remote m_remote;
    QString m_arguments;
    bool m_isLocal;
};



} // namespace Valgrind

#endif // VALGRINDPROCESS_H
