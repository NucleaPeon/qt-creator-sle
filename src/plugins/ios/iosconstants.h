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
#ifndef IOSCONSTANTS_H
#define IOSCONSTANTS_H

#include <QtGlobal>

namespace Ios {
namespace Internal {

} // namespace Internal

namespace IosDeviceType {
enum Enum {
    IosDevice,
    SimulatedIphone,
    SimulatedIpad,
    SimulatedIphoneRetina4Inch,
    SimulatedIphoneRetina3_5Inch,
    SimulatedIpadRetina
};
}

namespace Constants {
const char IOS_SETTINGS_ID[] = "ZZ.Ios Configurations";
const char IOS_SETTINGS_CATEGORY[] = "XA.Ios";
const char IOS_SETTINGS_TR_CATEGORY[] = QT_TRANSLATE_NOOP("Ios", "iOS");
const char IOS_SETTINGS_CATEGORY_ICON[] = ":/ios/images/iossettings.png";
const char IOSQT[] = "Qt4ProjectManager.QtVersion.Ios";

const char IOS_DEVICE_TYPE[] = "Ios.Device.Type";
const char IOS_SIMULATOR_TYPE[] = "Ios.Simulator.Type";
const char IOS_DEVICE_ID[] = "iOS Device ";
const char IOS_SIMULATOR_DEVICE_ID[] = "iOS Simulator Device ";
const char IOS_DSYM_BUILD_STEP_ID[] = "Ios.IosDsymBuildStep";

const quint16 IOS_DEVICE_PORT_START = 30000;
const quint16 IOS_DEVICE_PORT_END = 31000;
const quint16 IOS_SIMULATOR_PORT_START = 30000;
const quint16 IOS_SIMULATOR_PORT_END = 31000;

const char EXTRA_INFO_KEY[] = "extraInfo";

} // namespace Constants;
} // namespace Ios

#endif  // IOSCONSTANTS_H
