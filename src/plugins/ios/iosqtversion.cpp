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

#include "iosqtversion.h"
#include "iosconstants.h"
#include "iosconfigurations.h"
#include "iosmanager.h"

#include <utils/environment.h>
#include <utils/hostosinfo.h>

#include <qmakeprojectmanager/qmakeproject.h>
#include <qmakeprojectmanager/qmakeprojectmanagerconstants.h>

#include <qtsupport/qtkitinformation.h>
#include <qtsupport/qtsupportconstants.h>
#include <qtsupport/qtversionmanager.h>

#include <projectexplorer/kit.h>
#include <projectexplorer/projectexplorer.h>

using namespace Ios::Internal;
using namespace ProjectExplorer;
using namespace QmakeProjectManager;

IosQtVersion::IosQtVersion()
    : QtSupport::BaseQtVersion()
{
}

IosQtVersion::IosQtVersion(const Utils::FileName &path, bool isAutodetected,
                           const QString &autodetectionSource)
    : QtSupport::BaseQtVersion(path, isAutodetected, autodetectionSource)
{
    setDisplayName(defaultDisplayName(qtVersionString(), path, false));
}

IosQtVersion *IosQtVersion::clone() const
{
    return new IosQtVersion(*this);
}

QString IosQtVersion::type() const
{
    return QLatin1String(Constants::IOSQT);
}

bool IosQtVersion::isValid() const
{
    if (!BaseQtVersion::isValid())
        return false;
    if (qtAbis().isEmpty())
        return false;
    return true;
}

QString IosQtVersion::invalidReason() const
{
    QString tmp = BaseQtVersion::invalidReason();
    if (tmp.isEmpty() && qtAbis().isEmpty())
        return tr("Failed to detect the ABIs used by the Qt version.");
    return tmp;
}

QList<ProjectExplorer::Abi> IosQtVersion::detectQtAbis() const
{
    QList<ProjectExplorer::Abi> abis = qtAbisFromLibrary(qtCorePaths(versionInfo(), qtVersionString()));
    for (int i = 0; i < abis.count(); ++i) {
        abis[i] = Abi(abis.at(i).architecture(),
                      abis.at(i).os(),
                      ProjectExplorer::Abi::GenericMacFlavor,
                      abis.at(i).binaryFormat(),
                      abis.at(i).wordWidth());
    }
    return abis;
}

void IosQtVersion::addToEnvironment(const ProjectExplorer::Kit *k, Utils::Environment &env) const
{
    Q_UNUSED(k);
    Q_UNUSED(env);
}

QString IosQtVersion::description() const
{
    //: Qt Version is meant for Ios
    return tr("iOS");
}

Core::FeatureSet IosQtVersion::availableFeatures() const
{
    Core::FeatureSet features = QtSupport::BaseQtVersion::availableFeatures();
    features |= Core::FeatureSet(QtSupport::Constants::FEATURE_MOBILE);
    return features;
}

QString IosQtVersion::platformName() const
{
    return QLatin1String(QtSupport::Constants::IOS_PLATFORM);
}

QString IosQtVersion::platformDisplayName() const
{
    return QLatin1String(QtSupport::Constants::IOS_PLATFORM_TR);
}
