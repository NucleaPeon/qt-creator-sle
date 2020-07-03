/****************************************************************************
**
** Copyright (C) 2020 PeonDevelopments 
** Contact: Daniel Kettle <initial.dann@gmail.com>
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
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

#include "objectivecclasswizard.h"
#include "objectivecclasswizarddialog.h"
#include "objectivecclassnamepage.h"
#include "objectivecsourcegenerator.h"
#include "../objectiveceditorconstants.h"
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/target.h>
#include <qtsupport/qtkitinformation.h>
#include <qtsupport/baseqtversion.h>

using namespace ProjectExplorer;

namespace ObjectiveCEditor {
namespace Internal {

ClassWizard::ClassWizard()
{
    setWizardKind(Core::IWizard::FileWizard);
    setId(QLatin1String(Constants::C_OBJC_CLASS_WIZARD_ID));
    setCategory(QLatin1String(Constants::C_OBJC_WIZARD_CATEGORY));
    setDisplayCategory(QLatin1String(Constants::C_OBJC_DISPLAY_CATEGORY));
    setDisplayName(ClassWizard::tr(Constants::EN_OBJC_CLASS_DISPLAY_NAME));
    setDescription(ClassWizard::tr(Constants::EN_OBJC_CLASS_DESCRIPTION));
}

QWizard *ClassWizard::createWizardDialog(
        QWidget *parent,
        const Core::WizardDialogParameters &params) const
{
    ClassWizardDialog *wizard = new ClassWizardDialog(parent);
    foreach (QWizardPage *p, params.extensionPages())
        BaseFileWizard::applyExtensionPageShortTitle(wizard, wizard->addPage(p));
    wizard->setPath(params.defaultPath());
    wizard->setExtraValues(params.extraValues());
    return wizard;
}

Core::GeneratedFiles ClassWizard::generateFiles(const QWizard *w,
                                                QString *errorMessage) const
{
    Q_UNUSED(errorMessage);

    const ClassWizardDialog *wizard = qobject_cast<const ClassWizardDialog *>(w);
    const ClassWizardParameters params = wizard->parameters();

    const QString fileName = Core::BaseFileWizard::buildFileName(
                params.path, params.fileName, QLatin1String(Constants::C_OBJC_EXTENSION));
    Core::GeneratedFile sourceFile(fileName);

    SourceGenerator generator;
//    generator.setObjectiveCQtBinding(SourceGenerator::PySide);
    Kit *kit = kitForWizard(wizard);
    if (kit) {
        QtSupport::BaseQtVersion *baseVersion = QtSupport::QtKitInformation::qtVersion(kit);
        if (baseVersion && baseVersion->qtVersion().majorVersion == 5)
            generator.setObjectiveCQtVersion(SourceGenerator::Qt5);
        else
            generator.setObjectiveCQtVersion(SourceGenerator::Qt4);
    }

    QString sourceContent = generator.generateClass(
                params.className, params.baseClass, params.classType
                );

    sourceFile.setContents(sourceContent);
    sourceFile.setAttributes(Core::GeneratedFile::OpenEditorAttribute);
    return Core::GeneratedFiles() << sourceFile;
}

Kit *ClassWizard::kitForWizard(const ClassWizardDialog *wizard) const
{
    const QString key = QLatin1String(ProjectExplorer::Constants::PREFERED_PROJECT_NODE);
    const QString nodePath = wizard->extraValues().value(key).toString();

    // projectForFile doesn't find project if project file path passed
    Node *node = SessionManager::nodeForFile(nodePath);
    Project *proj = SessionManager::projectForNode(node);
    if (proj && proj->activeTarget())
        return proj->activeTarget()->kit();

    return KitManager::defaultKit();
}

} // namespace Internal
} // namespace ObjectiveCEditor
