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

#include "projectimporter.h"

#include "kit.h"
#include "kitmanager.h"
#include "project.h"

#include <coreplugin/idocument.h>

#include <utils/qtcassert.h>

namespace ProjectExplorer {

static const Core::Id KIT_IS_TEMPORARY("PE.TempKit");
static const Core::Id KIT_TEMPORARY_NAME("PE.TempName");
static const Core::Id KIT_FINAL_NAME("PE.FinalName");
static const Core::Id TEMPORARY_OF_PROJECTS("PE.TempProject");

ProjectImporter::ProjectImporter(const QString &path) : m_projectPath(path), m_isUpdating(false)
{ }

ProjectImporter::~ProjectImporter()
{
    foreach (Kit *k, KitManager::kits())
        removeProject(k, m_projectPath);
}

void ProjectImporter::markTemporary(Kit *k)
{
    QTC_ASSERT(!k->hasValue(KIT_IS_TEMPORARY), return);

    setIsUpdating(true);

    const QString name = k->displayName();
    k->setDisplayName(QCoreApplication::translate("ProjectExplorer::ProjectImporter",
                                                  "%1 - temporary").arg(name));

    k->setValue(KIT_TEMPORARY_NAME, k->displayName());
    k->setValue(KIT_FINAL_NAME, name);
    k->setValue(KIT_IS_TEMPORARY, true);

    setIsUpdating(false);
}

void ProjectImporter::makePermanent(Kit *k)
{
    if (!k->hasValue(KIT_IS_TEMPORARY))
        return;

    setIsUpdating(true);

    k->removeKey(KIT_IS_TEMPORARY);
    k->removeKey(TEMPORARY_OF_PROJECTS);
    const QString tempName = k->value(KIT_TEMPORARY_NAME).toString();
    if (!tempName.isNull() && k->displayName() == tempName)
        k->setDisplayName(k->value(KIT_FINAL_NAME).toString());
    k->removeKey(KIT_TEMPORARY_NAME);
    k->removeKey(KIT_FINAL_NAME);

    setIsUpdating(false);
}

void ProjectImporter::cleanupKit(Kit *k)
{
    Q_UNUSED(k);
}

void ProjectImporter::addProject(Kit *k)
{
    if (!k->hasValue(KIT_IS_TEMPORARY))
        return;

    QStringList projects = k->value(TEMPORARY_OF_PROJECTS, QStringList()).toStringList();

    projects.append(m_projectPath); // note: There can be more than one instance of the project added!

    setIsUpdating(true);

    k->setValue(TEMPORARY_OF_PROJECTS, projects);

    setIsUpdating(false);
}

void ProjectImporter::removeProject(Kit *k, const QString &path)
{
    if (!k->hasValue(KIT_IS_TEMPORARY))
        return;

    QStringList projects = k->value(TEMPORARY_OF_PROJECTS, QStringList()).toStringList();
    projects.removeOne(path);

    setIsUpdating(true);

    if (projects.isEmpty())
        ProjectExplorer::KitManager::deregisterKit(k);
    else
        k->setValue(TEMPORARY_OF_PROJECTS, projects);

    setIsUpdating(false);
}

} // namespace ProjectExplorer
