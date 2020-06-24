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

#include "objectivec.h"
#include "objectivecdialog.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <cpptools/abstracteditorsupport.h>
#include <qtsupport/qtsupportconstants.h>

#include <QCoreApplication>
#include <QFileInfo>
#include <QTextStream>

static const char mainCppC[] =
"#import <Foundation/Foundation.h>\n\n"
"int main(int argc, const char * argv[])\n"
"{\n"
"    @autoreleasepool {\n"
"        NSLog(@\"Hello, World!\");\n"
"    }\n"
"    return 0;\n"
"}\n";

static const char mainSourceFileC[] = "main";

namespace QmakeProjectManager {
namespace Internal {

ObjectiveCWizard::ObjectiveCWizard()
{
    setId(QLatin1String("C.ObjCPP"));
    setCategory(QLatin1String(ProjectExplorer::Constants::QT_APPLICATION_WIZARD_CATEGORY));
    setDisplayCategory(QCoreApplication::translate("ProjectExplorer",
             ProjectExplorer::Constants::QT_APPLICATION_WIZARD_CATEGORY_DISPLAY));
    setDisplayName(tr("Objective C/C++ Application"));
    setDescription(tr("Creates a project containing a single main.mm file with a stub implementation.\n\n"
                "Preselects a desktop Qt for building the application if available."));
    setIcon(QIcon(QLatin1String(":/wizards/images/console.png")));
    setRequiredFeatures(Core::Feature(QtSupport::Constants::FEATURE_QT_CONSOLE));
}

QWizard *ObjectiveCWizard::createWizardDialog(QWidget *parent,
                                              const Core::WizardDialogParameters &wizardDialogParameters) const
{
    ObjectiveCWizardDialog *dialog = new ObjectiveCWizardDialog(displayName(), icon(),
                                                                showModulesPageForApplications(), parent, wizardDialogParameters);
    dialog->setProjectName(ObjectiveCWizardDialog::uniqueProjectName(wizardDialogParameters.defaultPath()));
    return dialog;
}

Core::GeneratedFiles
        ObjectiveCWizard::generateFiles(const QWizard *w,
                                        QString * /*errorMessage*/) const
{
    const ObjectiveCWizardDialog *wizard = qobject_cast< const ObjectiveCWizardDialog *>(w);
    const QtProjectParameters params = wizard->parameters();
    const QString projectPath = params.projectPath();

    // Create files: Source

    const QString sourceFileName = Core::BaseFileWizard::buildFileName(projectPath, QLatin1String(mainSourceFileC), objcppSuffix());
    Core::GeneratedFile source(sourceFileName);
    source.setContents(CppTools::AbstractEditorSupport::licenseTemplate(sourceFileName) + QLatin1String(mainCppC));
    source.setAttributes(Core::GeneratedFile::OpenEditorAttribute);
    // Create files: Profile
    const QString profileName = Core::BaseFileWizard::buildFileName(projectPath, params.fileName, profileSuffix());

    Core::GeneratedFile profile(profileName);
    profile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);
    QString contents;
    {
        QTextStream proStr(&contents);
        QtProjectParameters::writeProFileHeader(proStr);
        params.writeProFile(proStr);
        proStr << "\n\nmacx {\n    LIBS += -framework Foundation\n    OBJECTIVE_SOURCES += "
               << QFileInfo(sourceFileName).fileName() << " \n}\n";
    }
    profile.setContents(contents);
    return Core::GeneratedFiles() <<  source << profile;
}

} // namespace Internal
} // namespace QmakeProjectManager
