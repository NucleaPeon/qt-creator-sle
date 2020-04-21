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

#include "osxappwizarddialog.h"
#include "filespage.h"

#include <qtsupport/qtsupportconstants.h>
#include <projectexplorer/projectexplorerconstants.h>

namespace QmakeProjectManager {
namespace Internal {

OSXAppWizardDialog::OSXAppWizardDialog(const QString &templateName,
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

    setIntroDescription(tr("This wizard generates an OS X Qt Application "
         "project. The application derives by default from QApplication "
         "and includes an empty widget."));

    addModulesPage();
    if (!parameters.extraValues().contains(QLatin1String(ProjectExplorer::Constants::PROJECT_KIT_IDS)))
        addTargetSetupPage();

    addExtensionPages(parameters.extensionPages());
}

void OSXAppWizardDialog::setBaseClasses(const QStringList &baseClasses)
{
    m_filesPage->setBaseClassChoices(baseClasses);
    if (!baseClasses.empty())
        m_filesPage->setBaseClassName(baseClasses.front());
}

void OSXAppWizardDialog::setSuffixes(const QString &header, const QString &source, const QString &form)
{
    m_filesPage->setSuffixes(header, source, form);
}

void OSXAppWizardDialog::setLowerCaseFiles(bool l)
{
    m_filesPage->setLowerCaseFiles(l);
}

QtProjectParameters OSXAppWizardDialog::projectParameters() const
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

GuiAppParameters OSXAppWizardDialog::parameters() const
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
