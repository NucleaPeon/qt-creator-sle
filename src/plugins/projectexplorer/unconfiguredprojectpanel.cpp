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

#include "unconfiguredprojectpanel.h"

#include "kit.h"
#include "kitmanager.h"
#include "project.h"
#include "projectexplorer.h"
#include "projectexplorerconstants.h"
#include "session.h"
#include "targetsetuppage.h"

#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/coreconstants.h>

#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

namespace ProjectExplorer {
namespace Internal {

UnconfiguredProjectPanel::UnconfiguredProjectPanel()
{
}

QString UnconfiguredProjectPanel::id() const
{
    return QLatin1String(Constants::UNCONFIGURED_PANEL_PAGE_ID);
}

QString UnconfiguredProjectPanel::displayName() const
{
    return tr("Configure Project");
}

int UnconfiguredProjectPanel::priority() const
{
    return -10;
}

bool UnconfiguredProjectPanel::supports(Project *project)
{
    return project->targets().isEmpty() && project->supportsNoTargetPanel();
}

PropertiesPanel *UnconfiguredProjectPanel::createPanel(Project *project)
{
    PropertiesPanel *panel = new PropertiesPanel;
    panel->setDisplayName(displayName());
    panel->setIcon(QIcon(QLatin1String(":/projectexplorer/images/unconfigured.png")));

    TargetSetupPageWrapper *w = new TargetSetupPageWrapper(project);
    panel->setWidget(w);
    return panel;
}

/////////
/// TargetSetupPageWrapper
////////

TargetSetupPageWrapper::TargetSetupPageWrapper(Project *project) :
    QWidget(), m_project(project)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    setLayout(layout);

    m_targetSetupPage = new TargetSetupPage(this);
    m_targetSetupPage->setProjectImporter(project->createProjectImporter());
    m_targetSetupPage->setUseScrollArea(false);
    m_targetSetupPage->setProjectPath(project->projectFilePath());
    m_targetSetupPage->setRequiredKitMatcher(project->createRequiredKitMatcher());
    m_targetSetupPage->setPreferredKitMatcher(project->createPreferredKitMatcher());
    m_targetSetupPage->initializePage();
    m_targetSetupPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    updateNoteText();

    layout->addWidget(m_targetSetupPage);

    // Apply row
    QHBoxLayout *hbox = new QHBoxLayout();
    layout->addLayout(hbox);
    layout->setMargin(0);
    hbox->addStretch();

    QDialogButtonBox *box = new QDialogButtonBox(this);

    m_configureButton = new QPushButton(this);
    m_configureButton->setText(tr("Configure Project"));
    box->addButton(m_configureButton, QDialogButtonBox::AcceptRole);

    m_cancelButton = new QPushButton(this);
    m_cancelButton->setText(tr("Cancel"));
    box->addButton(m_cancelButton, QDialogButtonBox::RejectRole);

    hbox->addWidget(box);

    layout->addStretch(10);

    completeChanged();

    connect(m_configureButton, SIGNAL(clicked()),
            this, SLOT(done()));
    connect(m_cancelButton, SIGNAL(clicked()),
            this, SLOT(cancel()));
    connect(m_targetSetupPage, SIGNAL(completeChanged()),
            this, SLOT(completeChanged()));
    connect(KitManager::instance(), SIGNAL(defaultkitChanged()),
            this, SLOT(updateNoteText()));
    connect(KitManager::instance(), SIGNAL(kitUpdated(ProjectExplorer::Kit*)),
            this, SLOT(kitUpdated(ProjectExplorer::Kit*)));
}

void TargetSetupPageWrapper::kitUpdated(Kit *k)
{
    if (k == KitManager::defaultKit())
        updateNoteText();
}

void TargetSetupPageWrapper::updateNoteText()
{
    Kit *k = KitManager::defaultKit();

    QString text;
    bool showHint = false;
    if (!k) {
        text = tr("The project <b>%1</b> is not yet configured.<br/>"
                  "Qt Creator cannot parse the project, because no kit "
                  "has been set up.")
                .arg(m_project->displayName());
        showHint = true;
    } else if (k->isValid()) {
        text = tr("The project <b>%1</b> is not yet configured.<br/>"
                  "Qt Creator uses the kit <b>%2</b> to parse the project.")
                .arg(m_project->displayName())
                .arg(k->displayName());
        showHint = false;
    } else {
        text = tr("The project <b>%1</b> is not yet configured.<br/>"
                  "Qt Creator uses the <b>invalid</b> kit <b>%2</b> to parse the project.")
                .arg(m_project->displayName())
                .arg(k->displayName());
        showHint = true;
    }

    m_targetSetupPage->setNoteText(text);
    m_targetSetupPage->showOptionsHint(showHint);
}

void TargetSetupPageWrapper::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->accept();
        done();
    }
}

void TargetSetupPageWrapper::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        event->accept();
}

void TargetSetupPageWrapper::cancel()
{
    ProjectExplorerPlugin::instance()->unloadProject(m_project);
    if (!SessionManager::hasProjects())
        Core::ModeManager::activateMode(Core::Constants::MODE_WELCOME);
}

void TargetSetupPageWrapper::done()
{
    m_targetSetupPage->setupProject(m_project);
    ProjectExplorerPlugin::instance()->requestProjectModeUpdate(m_project);
    Core::ModeManager::activateMode(Core::Constants::MODE_EDIT);
}

void TargetSetupPageWrapper::completeChanged()
{
    m_configureButton->setEnabled(m_targetSetupPage->isComplete());
}

} // namespace Internal
} // namespace ProjectExplorer
