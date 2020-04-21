/****************************************************************************
**
** Copyright (C) 2020 Daniel Kettle
** Contact: initial.dann@gmail.com
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

#include "osxfullappwizarddialog.h"
#include "filespage.h"

#include <qtsupport/qtsupportconstants.h>
#include <projectexplorer/projectexplorerconstants.h>

namespace QmakeProjectManager {
namespace Internal {

OSXFullAppWizardDialog::OSXFullAppWizardDialog(const QString &templateName,
                                       const QIcon &icon,
                                       bool showModulesPage,
                                       QWidget *parent,
                                       const Core::WizardDialogParameters &parameters) :
    BaseQmakeProjectWizardDialog(showModulesPage, parent, parameters),
    m_filesPage(new FilesPage)
{
    setWindowIcon(icon);
    setWindowTitle(templateName);
    setSelectedModules(QLatin1String("core gui"), true);

    setIntroDescription(tr("This wizard generates an enhanced OS X Qt Application "
         "project. The application derives by default from QApplication "
         "and includes menus, settings, empty modals and the main application area."));

    addModulesPage();
    if (!parameters.extraValues().contains(QLatin1String(ProjectExplorer::Constants::PROJECT_KIT_IDS)))
        addTargetSetupPage();

    addExtensionPages(parameters.extensionPages());
}

void OSXFullAppWizardDialog::setBaseClasses(const QStringList &baseClasses)
{
    m_filesPage->setBaseClassChoices(baseClasses);
    if (!baseClasses.empty())
        m_filesPage->setBaseClassName(baseClasses.front());
}

void OSXFullAppWizardDialog::setSuffixes(const QString &header, const QString &source, const QString &form)
{
    m_filesPage->setSuffixes(header, source, form);
}

void OSXFullAppWizardDialog::setLowerCaseFiles(bool l)
{
    m_filesPage->setLowerCaseFiles(l);
}

QtProjectParameters OSXFullAppWizardDialog::projectParameters() const
{
    QtProjectParameters rc;
    rc.type =  QtProjectParameters::GuiApp;
    rc.flags |= QtProjectParameters::WidgetsRequiredFlag;
    rc.fileName = projectName();
    rc.path = path();
    rc.selectedModules = selectedModulesList();
    rc.deselectedModules = deselectedModulesList();
    return rc;
}

GuiAppParameters OSXFullAppWizardDialog::parameters() const
{
    GuiAppParameters rc;
    rc.className = m_filesPage->className();
    rc.baseClassName = m_filesPage->baseClassName();
    rc.sourceFileName = m_filesPage->sourceFileName();
    rc.headerFileName = m_filesPage->headerFileName();
    rc.formFileName = m_filesPage->formFileName();
    rc.designerForm =  m_filesPage->formInputChecked();
    rc.isMobileApplication = false;
    return rc;
}

} // namespace Internal
} // namespace QmakeProjectManager
