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

#include "winrtdeployconfiguration.h"
#include "winrtpackagedeploymentstep.h"
#include "winrtconstants.h"

#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <qtsupport/qtkitinformation.h>

#include <QCoreApplication>

using namespace ProjectExplorer;

namespace WinRt {
namespace Internal {

static const char appxDeployConfigurationC[] = "WinRTAppxDeployConfiguration";
static const char phoneDeployConfigurationC[] = "WinRTPhoneDeployConfiguration";
static const char emulatorDeployConfigurationC[] = "WinRTEmulatorDeployConfiguration";

static QString msgDeployConfigurationDisplayName(const Core::Id &id)
{
    if (id == appxDeployConfigurationC) {
        return QCoreApplication::translate("WinRt::Internal::WinRtDeployConfiguration",
                                           "Run windeployqt");
    }
    if (id == phoneDeployConfigurationC) {
        return QCoreApplication::translate("WinRt::Internal::WinRtDeployConfiguration",
                                           "Deploy to Windows Phone");
    }
    if (id == emulatorDeployConfigurationC) {
        return QCoreApplication::translate("WinRt::Internal::WinRtDeployConfiguration",
                                           "Deploy to Windows Phone Emulator");
    }
    return QString();
}

WinRtDeployConfigurationFactory::WinRtDeployConfigurationFactory(QObject *parent)
    : DeployConfigurationFactory(parent)
{
}

QString WinRtDeployConfigurationFactory::displayNameForId(const Core::Id id) const
{
    return msgDeployConfigurationDisplayName(id);
}

QList<Core::Id> WinRtDeployConfigurationFactory::availableCreationIds(Target *parent) const
{
    if (!parent->project()->supportsKit(parent->kit()))
        return QList<Core::Id>();

    IDevice::ConstPtr device = DeviceKitInformation::device(parent->kit());
    if (!device)
        return QList<Core::Id>();

    if (device->type() == Constants::WINRT_DEVICE_TYPE_LOCAL)
        return QList<Core::Id>() << Core::Id(appxDeployConfigurationC);

    if (device->type() == Constants::WINRT_DEVICE_TYPE_PHONE)
        return QList<Core::Id>() << Core::Id(phoneDeployConfigurationC);

    if (device->type() == Constants::WINRT_DEVICE_TYPE_EMULATOR)
        return QList<Core::Id>() << Core::Id(emulatorDeployConfigurationC);

    return QList<Core::Id>();
}

bool WinRtDeployConfigurationFactory::canCreate(Target *parent, const Core::Id id) const
{
    return availableCreationIds(parent).contains(id);
}

DeployConfiguration *WinRtDeployConfigurationFactory::create(Target *parent, const Core::Id id)
{
    if (id == appxDeployConfigurationC
            || id == phoneDeployConfigurationC
            || id == emulatorDeployConfigurationC) {
        return new WinRtDeployConfiguration(parent, id);
    }
    return 0;
}

bool WinRtDeployConfigurationFactory::canRestore(Target *parent, const QVariantMap &map) const
{
    return canCreate(parent, idFromMap(map));
}

DeployConfiguration *WinRtDeployConfigurationFactory::restore(Target *parent,
                                                              const QVariantMap &map)
{
    WinRtDeployConfiguration *dc = new WinRtDeployConfiguration(parent, idFromMap(map));
    if (!dc->fromMap(map)) {
        delete dc;
        return 0;
    }
    return dc;
}

bool WinRtDeployConfigurationFactory::canClone(Target *parent, DeployConfiguration *source) const
{
    return availableCreationIds(parent).contains(source->id());
}

DeployConfiguration *WinRtDeployConfigurationFactory::clone(Target *parent,
                                                            DeployConfiguration *source)
{
    if (source->id() == appxDeployConfigurationC
            || source->id() == phoneDeployConfigurationC
            || source->id() == emulatorDeployConfigurationC) {
        return new WinRtDeployConfiguration(parent, source);
    }
    return 0;
}

QList<Core::Id> WinRtDeployStepFactory::availableCreationIds(BuildStepList *parent) const
{
    QList<Core::Id> ids;
    if (parent->id() != ProjectExplorer::Constants::BUILDSTEPS_DEPLOY)
        return ids;
    if (!parent->target()->project()->supportsKit(parent->target()->kit()))
        return ids;
    if (!parent->contains(Constants::WINRT_BUILD_STEP_DEPLOY))
        ids << Constants::WINRT_BUILD_STEP_DEPLOY;
    return ids;
}

QString WinRtDeployStepFactory::displayNameForId(const Core::Id id) const
{
    if (id == Constants::WINRT_BUILD_STEP_DEPLOY) {
        return QCoreApplication::translate("WinRt::Internal::WinRtDeployStepFactory",
                                           "Run windeployqt");
    }
    return QString();
}

bool WinRtDeployStepFactory::canCreate(BuildStepList *parent, const Core::Id id) const
{
    return availableCreationIds(parent).contains(id);
}

BuildStep *WinRtDeployStepFactory::create(BuildStepList *parent, const Core::Id id)
{
    if (id == Constants::WINRT_BUILD_STEP_DEPLOY)
        return new WinRtPackageDeploymentStep(parent);
    return 0;
}

bool WinRtDeployStepFactory::canRestore(BuildStepList *parent, const QVariantMap &map) const
{
    return canCreate(parent, idFromMap(map));
}

BuildStep *WinRtDeployStepFactory::restore(BuildStepList *parent, const QVariantMap &map)
{
    BuildStep *bs = create(parent, idFromMap(map));
    if (!bs->fromMap(map)) {
        delete bs;
        bs = 0;
    }
    return bs;
}

bool WinRtDeployStepFactory::canClone(BuildStepList *parent, BuildStep *source) const
{
    Q_UNUSED(parent);
    Q_UNUSED(source);
    return false;
}

BuildStep *WinRtDeployStepFactory::clone(BuildStepList *parent, BuildStep *source)
{
    Q_UNUSED(parent);
    Q_UNUSED(source);
    return 0;
}

WinRtDeployConfiguration::WinRtDeployConfiguration(Target *target, Core::Id id)
    : DeployConfiguration(target, id)
{
    setDefaultDisplayName(msgDeployConfigurationDisplayName(id));

    stepList()->insertStep(0, new WinRtPackageDeploymentStep(stepList()));
}

WinRtDeployConfiguration::WinRtDeployConfiguration(Target *target, DeployConfiguration *source)
    : DeployConfiguration(target, source)
{
    cloneSteps(source);
}

} // namespace Internal
} // namespace WinRt
