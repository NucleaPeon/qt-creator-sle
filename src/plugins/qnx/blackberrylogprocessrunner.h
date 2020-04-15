/**************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
**
** Contact: BlackBerry (qt@blackberry.com)
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

#ifndef BLACKBERRYSLOGPROCESS_H
#define BLACKBERRYSLOGPROCESS_H

#include "blackberrydeviceconfiguration.h"

#include <utils/outputformat.h>

#include <QObject>

namespace ProjectExplorer { class SshDeviceProcess; }

namespace Qnx {
namespace Internal {

class Slog2InfoRunner;

class BlackBerryLogProcessRunner : public QObject
{
    Q_OBJECT
public:
    explicit BlackBerryLogProcessRunner(QObject *parent, const QString &appId, const BlackBerryDeviceConfiguration::ConstPtr &device);
    void start();

signals:
    void output(const QString &msg, Utils::OutputFormat format);
    void finished();

public slots:
    void stop();

private slots:
    void launchTailProcess();
    void readTailStandardOutput();
    void readTailStandardError();
    void handleTailProcessError();

private:
    QString m_appId;

    BlackBerryDeviceConfiguration::ConstPtr m_device;
    ProjectExplorer::SshDeviceProcess *m_tailProcess;
    Slog2InfoRunner *m_slog2InfoRunner;
};
} // namespace Internal
} // namespace Qnx

#endif // BLACKBERRYSLOGPROCESS_H
