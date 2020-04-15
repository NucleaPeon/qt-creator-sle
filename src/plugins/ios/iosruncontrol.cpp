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

#include "iosruncontrol.h"

#include "iosrunconfiguration.h"
#include "iosrunner.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <QIcon>

using namespace ProjectExplorer;

namespace Ios {
namespace Internal {

IosRunControl::IosRunControl(IosRunConfiguration *rc)
    : RunControl(rc, NormalRunMode)
    , m_runner(new IosRunner(this, rc, false, false))
    , m_running(false)
{
}

IosRunControl::~IosRunControl()
{
    stop();
}

void IosRunControl::start()
{
    m_running = true;
    emit started();
    disconnect(m_runner, 0, this, 0);

    connect(m_runner, SIGNAL(errorMsg(QString)),
        SLOT(handleRemoteErrorOutput(QString)));
    connect(m_runner, SIGNAL(appOutput(QString)),
        SLOT(handleRemoteOutput(QString)));
    connect(m_runner, SIGNAL(finished(bool)),
        SLOT(handleRemoteProcessFinished(bool)));
    appendMessage(tr("Starting remote process.") + QLatin1Char('\n'), Utils::NormalMessageFormat);
    m_runner->start();
}

RunControl::StopResult IosRunControl::stop()
{
    m_runner->stop();
    return StoppedSynchronously;
}

void IosRunControl::handleRemoteProcessFinished(bool cleanEnd)
{
    if (!cleanEnd)
        appendMessage(tr("Run ended unexpectedly."), Utils::ErrorMessageFormat);
    disconnect(m_runner, 0, this, 0);
    m_running = false;
    emit finished();
}

void IosRunControl::handleRemoteOutput(const QString &output)
{
    appendMessage(output, Utils::StdOutFormatSameLine);
}

void IosRunControl::handleRemoteErrorOutput(const QString &output)
{
    appendMessage(output, Utils::StdErrFormat);
}

bool IosRunControl::isRunning() const
{
    return m_running;
}

QString IosRunControl::displayName() const
{
    return m_runner->displayName();
}

QIcon IosRunControl::icon() const
{
    return QIcon(QLatin1String(ProjectExplorer::Constants::ICON_DEBUG_SMALL));
}

} // namespace Internal
} // namespace Ios
