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

#include "iosdevicefactory.h"
#include "iosdevice.h"

#include "iosconstants.h"

namespace Ios {
namespace Internal {

IosDeviceFactory::IosDeviceFactory()
{
    setObjectName(QLatin1String("IosDeviceFactory"));
}

QString IosDeviceFactory::displayNameForId(Core::Id type) const
{
    return type == Constants::IOS_DEVICE_TYPE ? IosDevice::name() : QString();
}

QList<Core::Id> IosDeviceFactory::availableCreationIds() const
{
    return QList<Core::Id>() << Core::Id(Constants::IOS_DEVICE_TYPE);
}

bool IosDeviceFactory::canCreate() const
{
    return false;
}

ProjectExplorer::IDevice::Ptr IosDeviceFactory::create(Core::Id id) const
{
    Q_UNUSED(id)
    return ProjectExplorer::IDevice::Ptr();
}

bool IosDeviceFactory::canRestore(const QVariantMap &map) const
{
    if (ProjectExplorer::IDevice::typeFromMap(map) != Constants::IOS_DEVICE_TYPE)
        return false;
    QVariantMap vMap = map.value(QLatin1String(Constants::EXTRA_INFO_KEY)).toMap();
    if (vMap.isEmpty()
            || vMap.value(QLatin1String("deviceName")).toString() == QLatin1String("*unknown*"))
        return false; // transient device (probably generated during an activation)
    return true;
}

ProjectExplorer::IDevice::Ptr IosDeviceFactory::restore(const QVariantMap &map) const
{
    IosDevice *newDev = new IosDevice;
    newDev->fromMap(map);
    // updating the active ones should be enough...
    //IosDeviceManager::instance()->updateInfo(newDev->uniqueDeviceID());
    return ProjectExplorer::IDevice::Ptr(newDev);
}

} // namespace Internal
} // namespace Ios
