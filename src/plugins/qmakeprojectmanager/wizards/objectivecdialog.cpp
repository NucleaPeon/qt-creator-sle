/****************************************************************************
**
** Copyright (C) 2020 Daniel Kettle <initial.dann@gmail.com>
** Contact: https://github.com/NucleaPeon/qt-creator-sle
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

#include "objectivecdialog.h"
#include <projectexplorer/projectexplorerconstants.h>

#include <QDebug>

namespace QmakeProjectManager {
namespace Internal {

ObjectiveCWizardDialog::ObjectiveCWizardDialog(const QString &templateName,
                                               const QIcon &icon,
                                               bool showModulesPage,
                                               QWidget *parent, const Core::WizardDialogParameters &parameters) :
    BaseQmakeProjectWizardDialog(showModulesPage, parent, parameters)
{
    setWindowIcon(icon);
    setWindowTitle(templateName);
    setSelectedModules(QLatin1String("core"));
    setDeselectedModules(QLatin1String("gui"));

    setIntroDescription(tr("This wizard generates an Objective C/C++ Console Application "
                          "project. The application derives from the Foundation Framework "
                          "and does not provide a GUI."));

    addModulesPage();
    if (!parameters.extraValues().contains(QLatin1String(ProjectExplorer::Constants::PROJECT_KIT_IDS)))
        addTargetSetupPage();

    addExtensionPages(parameters.extensionPages());
}

QtProjectParameters ObjectiveCWizardDialog::parameters() const
{
    QtProjectParameters rc;
    rc.type = QtProjectParameters::ConsoleApp;
    rc.fileName = projectName();
    rc.path = path();

    rc.selectedModules = selectedModulesList();
    rc.deselectedModules = deselectedModulesList();
    return rc;
}

} // namespace Internal
} // namespace QmakeProjectManager
