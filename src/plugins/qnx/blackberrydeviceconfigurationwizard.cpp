/**************************************************************************
**
** Copyright (C) 2012 - 2014 BlackBerry Limited. All rights reserved.
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

#include "blackberrydeviceconfigurationwizard.h"
#include "blackberrydeviceconfigurationwizardpages.h"
#include "qnxconstants.h"
#include "blackberrydeviceconfiguration.h"
#include "blackberrydeviceconnectionmanager.h"

#include <ssh/sshconnection.h>

using namespace Qnx;
using namespace Qnx::Internal;

BlackBerryDeviceConfigurationWizard::BlackBerryDeviceConfigurationWizard(QWidget *parent) :
    Utils::Wizard(parent)
{
    setWindowTitle(tr("New BlackBerry Device Configuration Setup"));

    m_setupPage = new BlackBerryDeviceConfigurationWizardSetupPage(this);
    m_queryPage = new BlackBerryDeviceConfigurationWizardQueryPage(m_holder, this);
    m_configPage = new BlackBerryDeviceConfigurationWizardConfigPage(m_holder, this);
    m_finalPage = new BlackBerryDeviceConfigurationWizardFinalPage(this);

    setPage(SetupPageId, m_setupPage);
    setPage(QueryPageId, m_queryPage);
    setPage(ConfigPageId, m_configPage);
    setPage(FinalPageId, m_finalPage);
    m_finalPage->setCommitPage(true);
}

ProjectExplorer::IDevice::Ptr BlackBerryDeviceConfigurationWizard::device()
{
    QSsh::SshConnectionParameters sshParams;
    sshParams.options = QSsh::SshIgnoreDefaultProxy;
    sshParams.host = m_setupPage->hostName();
    sshParams.password = m_setupPage->password();
    sshParams.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePublicKey;
    sshParams.privateKeyFile = BlackBerryDeviceConnectionManager::instance()->privateKeyPath();
    sshParams.userName = QLatin1String("devuser");
    sshParams.timeout = 10;
    sshParams.port = 22;

    BlackBerryDeviceConfiguration::Ptr configuration = BlackBerryDeviceConfiguration::create(
            m_configPage->configurationName(),
            Core::Id(Constants::QNX_BB_OS_TYPE),
            m_holder.isSimulator
                    ? ProjectExplorer::IDevice::Emulator
                    : ProjectExplorer::IDevice::Hardware);
    configuration->setSshParameters(sshParams);
    configuration->setDebugToken(m_configPage->debugToken());
    return configuration;
}
