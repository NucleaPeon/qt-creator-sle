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

#ifndef QMAKEKITINFORMATION_H
#define QMAKEKITINFORMATION_H

#include "qmakeprojectmanager_global.h"

#include <projectexplorer/kitmanager.h>

namespace QmakeProjectManager {

class QMAKEPROJECTMANAGER_EXPORT QmakeKitInformation : public ProjectExplorer::KitInformation
{
    Q_OBJECT

public:
    QmakeKitInformation();

    QVariant defaultValue(ProjectExplorer::Kit *k) const;

    QList<ProjectExplorer::Task> validate(const ProjectExplorer::Kit *k) const;
    void setup(ProjectExplorer::Kit *k);

    ProjectExplorer::KitConfigWidget *createConfigWidget(ProjectExplorer::Kit *k) const;

    ItemList toUserOutput(const ProjectExplorer::Kit *k) const;

    static Core::Id id();
    static void setMkspec(ProjectExplorer::Kit *k, const Utils::FileName &fn);
    static Utils::FileName mkspec(const ProjectExplorer::Kit *k);
    static Utils::FileName effectiveMkspec(const ProjectExplorer::Kit *k);
    static Utils::FileName defaultMkspec(const ProjectExplorer::Kit *k);
};

} // namespace QmakeProjectManager

#endif // QMAKEKITINFORMATION_H
