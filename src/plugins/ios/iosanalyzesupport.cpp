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

#include "iosanalyzesupport.h"

#include "iosrunner.h"
#include "iosmanager.h"
#include "iosdevice.h"

#include <debugger/debuggerengine.h>
#include <debugger/debuggerplugin.h>
#include <debugger/debuggerkitinformation.h>
#include <debugger/debuggerrunner.h>
#include <debugger/debuggerstartparameters.h>
#include <debugger/debuggerrunconfigurationaspect.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/target.h>
#include <projectexplorer/taskhub.h>
#include <qmakeprojectmanager/qmakebuildconfiguration.h>
#include <qmakeprojectmanager/qmakenodes.h>
#include <qmakeprojectmanager/qmakeproject.h>
#include <qtsupport/qtkitinformation.h>
#include <utils/fileutils.h>
#include <analyzerbase/ianalyzertool.h>
#include <analyzerbase/analyzermanager.h>
#include <analyzerbase/analyzerruncontrol.h>
#include <analyzerbase/analyzerstartparameters.h>
#include <utils/outputformat.h>
#include <utils/qtcprocess.h>

#include <QDir>
#include <QTcpServer>
#include <QSettings>

#include <stdio.h>
#include <fcntl.h>
#ifdef Q_OS_UNIX
#include <unistd.h>
#else
#include <io.h>
#endif

using namespace Analyzer;
using namespace ProjectExplorer;
using namespace QmakeProjectManager;

namespace Ios {
namespace Internal {

RunControl *IosAnalyzeSupport::createAnalyzeRunControl(IosRunConfiguration *runConfig,
                                                       QString *errorMessage)
{
    Q_UNUSED(errorMessage);
    Target *target = runConfig->target();
    if (!target)
        return 0;
    ProjectExplorer::IDevice::ConstPtr device = DeviceKitInformation::device(target->kit());
    if (device.isNull())
        return 0;
    AnalyzerStartParameters params;
    params.runMode = QmlProfilerRunMode;
    params.sysroot = SysRootKitInformation::sysRoot(target->kit()).toString();
    params.startMode = StartLocal;
    params.debuggee = runConfig->exePath().toUserOutput();
    params.debuggeeArgs = Utils::QtcProcess::joinArgs(runConfig->commandLineArguments());
    params.analyzerHost = QLatin1String("localhost");
    if (device->type() == Core::Id(Ios::Constants::IOS_DEVICE_TYPE)) {
        IosDevice::ConstPtr iosDevice = device.dynamicCast<const IosDevice>();
        if (iosDevice.isNull())
                return 0;
    }
    params.displayName = runConfig->appName();

    AnalyzerRunControl *analyzerRunControl = AnalyzerManager::createRunControl(params, runConfig);
    (void) new IosAnalyzeSupport(runConfig, analyzerRunControl, false, true);
    return analyzerRunControl;
}

IosAnalyzeSupport::IosAnalyzeSupport(IosRunConfiguration *runConfig,
    AnalyzerRunControl *runControl, bool cppDebug, bool qmlDebug)
    : QObject(runControl), m_runControl(runControl),
      m_runner(new IosRunner(this, runConfig, cppDebug, qmlDebug))
{
    connect(m_runControl, SIGNAL(starting(const Analyzer::AnalyzerRunControl*)),
            m_runner, SLOT(start()));
    connect(m_runControl, SIGNAL(finished()),
            m_runner, SLOT(stop()));
    connect(&m_outputParser, SIGNAL(waitingForConnectionOnPort(quint16)),
            SLOT(qmlServerReady()));

    connect(m_runner, SIGNAL(gotServerPorts(int,int)),
        SLOT(handleServerPorts(int,int)));
    connect(m_runner, SIGNAL(gotInferiorPid(Q_PID, int)),
        SLOT(handleGotInferiorPid(Q_PID, int)));
    connect(m_runner, SIGNAL(finished(bool)),
        SLOT(handleRemoteProcessFinished(bool)));

    connect(m_runner, SIGNAL(errorMsg(QString)),
        SLOT(handleRemoteErrorOutput(QString)));
    connect(m_runner, SIGNAL(appOutput(QString)),
        SLOT(handleRemoteOutput(QString)));
}

IosAnalyzeSupport::~IosAnalyzeSupport()
{
}

void IosAnalyzeSupport::qmlServerReady()
{
    m_runControl->notifyRemoteSetupDone(m_qmlPort);
}

void IosAnalyzeSupport::handleServerPorts(int gdbServerPort, int qmlPort)
{
    Q_UNUSED(gdbServerPort);
    m_qmlPort = qmlPort;
}

void IosAnalyzeSupport::handleGotInferiorPid(Q_PID pid, int qmlPort)
{
    Q_UNUSED(pid);
    m_qmlPort = qmlPort;
}

void IosAnalyzeSupport::handleRemoteProcessFinished(bool cleanEnd)
{
    Q_UNUSED(cleanEnd)
    if (m_runControl) {
        m_runControl->logApplicationMessage(tr("Run ended."), Utils::NormalMessageFormat);
        m_runControl->notifyRemoteFinished();
    }
}

void IosAnalyzeSupport::handleRemoteOutput(const QString &output)
{
    if (m_runControl) {
        m_runControl->logApplicationMessage(output, Utils::StdOutFormat);
        m_outputParser.processOutput(output);
    }
}

void IosAnalyzeSupport::handleRemoteErrorOutput(const QString &output)
{
    if (m_runControl)
        m_runControl->logApplicationMessage(output, Utils::StdErrFormat);
}

} // namespace Internal
} // namespace Ios
