/**************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
**
** Contact: BlackBerry Limited (blackberry-qt@qnx.com)
** Contact: KDAB (info@kdab.com)
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

#include "qnxdeviceprocesslist.h"

#include <QRegExp>
#include <QStringList>

using namespace Qnx;
using namespace Qnx::Internal;

QnxDeviceProcessList::QnxDeviceProcessList(
        const ProjectExplorer::IDevice::ConstPtr &device, QObject *parent)
    : ProjectExplorer::SshDeviceProcessList(device, parent)
{
}

QString QnxDeviceProcessList::listProcessesCommandLine() const
{
    return QLatin1String("pidin -F \"%a %A '/%n'\"");
}

QList<ProjectExplorer::DeviceProcessItem> QnxDeviceProcessList::buildProcessList(
        const QString &listProcessesReply) const
{
    QList<ProjectExplorer::DeviceProcessItem> processes;
    QStringList lines = listProcessesReply.split(QLatin1Char('\n'));
    if (lines.isEmpty())
        return processes;

    lines.pop_front(); // drop headers
    QRegExp re(QLatin1String("\\s*(\\d+)\\s+(.*)'(.*)'"));

    foreach (const QString& line, lines) {
        if (re.exactMatch(line)) {
            const QStringList captures = re.capturedTexts();
            if (captures.size() == 4) {
                const int pid = captures[1].toInt();
                const QString args = captures[2];
                const QString exe = captures[3];
                ProjectExplorer::DeviceProcessItem deviceProcess;
                deviceProcess.pid = pid;
                deviceProcess.exe = exe.trimmed();
                deviceProcess.cmdLine = args.trimmed();
                processes.append(deviceProcess);
            }
        }
    }

    qSort(processes);
    return processes;
}
