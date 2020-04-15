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

#include "iosqtversionfactory.h"
#include "iosqtversion.h"
#include "iosconstants.h"
#include <qtsupport/qtsupportconstants.h>
#include <utils/qtcassert.h>
#include <proparser/profileevaluator.h>

#include <QFileInfo>
#include <QDebug>

namespace Ios {
namespace Internal {

IosQtVersionFactory::IosQtVersionFactory(QObject *parent)
    : QtSupport::QtVersionFactory(parent)
{
}

bool IosQtVersionFactory::canRestore(const QString &type)
{
    return type == QLatin1String(Constants::IOSQT);
}

QtSupport::BaseQtVersion *IosQtVersionFactory::restore(const QString &type,
    const QVariantMap &data)
{
    QTC_ASSERT(canRestore(type), return 0);
    IosQtVersion *v = new IosQtVersion;
    v->fromMap(data);
    return v;
}

int IosQtVersionFactory::priority() const
{
    return 90;
}

QtSupport::BaseQtVersion *IosQtVersionFactory::create(const Utils::FileName &qmakePath,
                                                      ProFileEvaluator *evaluator,
                                                      bool isAutoDetected,
                                                      const QString &autoDetectionSource)
{
    if (!(evaluator->values(QLatin1String("QMAKE_PLATFORM")).contains(QLatin1String("ios"))))
        return 0;
    return new IosQtVersion(qmakePath, isAutoDetected, autoDetectionSource);
}

} // Internal
} // Ios
