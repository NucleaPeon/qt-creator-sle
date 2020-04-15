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

#include "clangprojectsettings.h"
#include "clangprojectsettingspropertiespage.h"
#include "pchmanager.h"

#include <QButtonGroup>
#include <QCoreApplication>
#include <QFileDialog>

using namespace ProjectExplorer;
using namespace ClangCodeModel::Internal;

static const char CLANGPROJECTSETTINGS_PANEL_ID[] = "ClangCodeModel.ProjectPanel";


QString ClangProjectSettingsPanelFactory::id() const
{
    return QLatin1String(CLANGPROJECTSETTINGS_PANEL_ID);
}

QString ClangProjectSettingsPanelFactory::displayName() const
{
    return ClangProjectSettingsWidget::tr("Clang Settings");
}

int ClangProjectSettingsPanelFactory::priority() const
{
    return 60;
}

bool ClangProjectSettingsPanelFactory::supports(Project *project)
{
    Q_UNUSED(project);

    return true;
}

PropertiesPanel *ClangProjectSettingsPanelFactory::createPanel(Project *project)
{
    PropertiesPanel *panel = new PropertiesPanel;
    panel->setDisplayName(ClangProjectSettingsWidget::tr("Clang Settings"));
    panel->setWidget(new ClangProjectSettingsWidget(project));
    return panel;
}

ClangProjectSettingsWidget::ClangProjectSettingsWidget(Project *project)
    : m_project(project)
{
    m_ui.setupUi(this);

    ClangProjectSettings *cps = PchManager::instance()->settingsForProject(project);
    Q_ASSERT(cps);

    QButtonGroup *pchGroup = new QButtonGroup(this);
    pchGroup->addButton(m_ui.noneButton, ClangProjectSettings::PchUse_None);
    pchGroup->addButton(m_ui.exactButton, ClangProjectSettings::PchUse_BuildSystem_Exact);
    pchGroup->addButton(m_ui.fuzzyButton, ClangProjectSettings::PchUse_BuildSystem_Fuzzy);
    pchGroup->addButton(m_ui.customButton, ClangProjectSettings::PchUse_Custom);
    switch (cps->pchUsage()) {
    case ClangProjectSettings::PchUse_None:
    case ClangProjectSettings::PchUse_BuildSystem_Exact:
    case ClangProjectSettings::PchUse_BuildSystem_Fuzzy:
    case ClangProjectSettings::PchUse_Custom:
        pchGroup->button(cps->pchUsage())->setChecked(true);
        break;
    default: break;
    }
    pchUsageChanged(cps->pchUsage());
    connect(pchGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(pchUsageChanged(int)));

    m_ui.customField->setText(cps->customPchFile());
    connect(m_ui.customField, SIGNAL(editingFinished()),
            this, SLOT(customPchFileChanged()));
    connect(m_ui.customButton, SIGNAL(clicked()),
            this, SLOT(customPchButtonClicked()));
}

void ClangProjectSettingsWidget::pchUsageChanged(int id)
{
    ClangProjectSettings *cps = PchManager::instance()->settingsForProject(m_project);
    Q_ASSERT(cps);
    cps->setPchUsage(static_cast<ClangProjectSettings::PchUsage>(id));

    switch (id) {
    case ClangProjectSettings::PchUse_None:
    case ClangProjectSettings::PchUse_BuildSystem_Fuzzy:
    case ClangProjectSettings::PchUse_BuildSystem_Exact:
        m_ui.customField->setEnabled(false);
        m_ui.chooseButton->setEnabled(false);
        break;

    case ClangProjectSettings::PchUse_Custom:
        m_ui.customField->setEnabled(true);
        m_ui.chooseButton->setEnabled(true);
        break;

    default:
        break;
    }
}

void ClangProjectSettingsWidget::customPchFileChanged()
{
    ClangProjectSettings *cps = PchManager::instance()->settingsForProject(m_project);
    Q_ASSERT(cps);
    if (cps->pchUsage() != ClangProjectSettings::PchUse_Custom)
        return;
    QString fileName = m_ui.customField->text();
    if (!QFile(fileName).exists())
        return;

    cps->setCustomPchFile(fileName);
}

void ClangProjectSettingsWidget::customPchButtonClicked()
{
    ClangProjectSettings *cps = PchManager::instance()->settingsForProject(m_project);
    Q_ASSERT(cps);

    QFileDialog d(this);
    d.setNameFilters(QStringList() << tr("Header Files (*.h)")
                     << tr("All Files (*)"));
    d.setFileMode(QFileDialog::ExistingFile);
    d.setDirectory(m_project->projectDirectory());
    if (!d.exec())
        return;
    const QStringList fileNames = d.selectedFiles();
    if (fileNames.isEmpty() || fileNames.first().isEmpty())
        return;

    m_ui.customField->setText(fileNames.first());
    cps->setCustomPchFile(fileNames.first());
}
