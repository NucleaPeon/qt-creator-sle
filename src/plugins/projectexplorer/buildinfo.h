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

#ifndef BUILDINFO_H
#define BUILDINFO_H

#include "projectexplorer_export.h"

#include "task.h"

#include <coreplugin/id.h>
#include <utils/fileutils.h>

namespace ProjectExplorer {

class IBuildConfigurationFactory;

class PROJECTEXPLORER_EXPORT BuildInfo
{
public:
    BuildInfo(const IBuildConfigurationFactory *f) : supportsShadowBuild(false), m_factory(f) { }
    virtual ~BuildInfo() { }

    const IBuildConfigurationFactory *factory() const { return m_factory; }

    QString displayName;
    QString typeName;
    Utils::FileName buildDirectory;
    Core::Id kitId;
    bool supportsShadowBuild;

    virtual QList<Task> reportIssues(const QString &projectPath,
                                     const QString &buildDir) const
    {
        Q_UNUSED(projectPath);
        Q_UNUSED(buildDir);
        return QList<Task>();
    }

private:
    const IBuildConfigurationFactory *m_factory;

    friend class IBuildConfigurationFactory;
};

} // namespace ProjectExplorer

#endif // BUILDINFO_H
