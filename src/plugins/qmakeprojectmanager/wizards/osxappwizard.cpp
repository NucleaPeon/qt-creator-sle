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

#include "osxappwizard.h"
#include "osxappwizarddialog.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <cpptools/abstracteditorsupport.h>
#include <designer/cpp/formclasswizardparameters.h>
#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/invoker.h>
#include <qtsupport/qtsupportconstants.h>

#include <utils/fileutils.h>

#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QSharedPointer>

static const char mainSourceFileC[] = "main";
static const char mainWindowFileC[] = "mainwindow";
static const char cocoaC[] = "cocoainitializer";
static const char mainSourceShowC[] = "    w.show();\n";
static const char mainSourceMobilityShowC[] = "    w.show();\n";

static const char mainWindowUiContentsC[] =
"\n  <widget class=\"QMenuBar\" name=\"menuBar\" />"
"\n  <widget class=\"QToolBar\" name=\"mainToolBar\" />"
"\n  <widget class=\"QWidget\" name=\"centralWidget\" />"
"\n  <widget class=\"QStatusBar\" name=\"statusBar\" />";
static const char mainWindowMobileUiContentsC[] =
"\n  <widget class=\"QWidget\" name=\"centralWidget\" />";

static const char *baseClassesC[] = { "QMainWindow", "QWidget", "QDialog" };

static inline QStringList baseClasses()
{
    QStringList rc;
    const int baseClassCount = sizeof(baseClassesC)/sizeof(const char *);
    for (int i = 0; i < baseClassCount; i++)
        rc.push_back(QLatin1String(baseClassesC[i]));
    return rc;
}

namespace QmakeProjectManager {
namespace Internal {

OSXAppWizard::OSXAppWizard()
{
    setId(QLatin1String("C.OSXGui"));
    setCategory(QLatin1String(ProjectExplorer::Constants::QT_APPLICATION_WIZARD_CATEGORY));
    setDisplayCategory(QCoreApplication::translate("ProjectExplorer",
               ProjectExplorer::Constants::QT_APPLICATION_WIZARD_CATEGORY_DISPLAY));
    setDisplayName(tr("OSX Qt Application"));
    setDescription(tr("Creates a Cocoa-compatible Qt application for OS X. "
                  "Includes a Qt Designer-based main window.\n\n"
                  "Preselects a desktop Qt for building the application if available."));
    setIcon(QIcon(QLatin1String(":/wizards/images/osxgui.png")));
    setRequiredFeatures(Core::Feature(QtSupport::Constants::FEATURE_QWIDGETS));
}

QWizard *OSXAppWizard::createWizardDialog(QWidget *parent,
                                          const Core::WizardDialogParameters &wizardDialogParameters) const
{
    OSXAppWizardDialog *dialog = new OSXAppWizardDialog(displayName(), icon(),
                                                        showModulesPageForApplications(),
                                                        parent, wizardDialogParameters);
    dialog->setProjectName(OSXAppWizardDialog::uniqueProjectName(wizardDialogParameters.defaultPath()));
    // Order! suffixes first to generate files correctly
    dialog->setLowerCaseFiles(QtWizard::lowerCaseFiles());
    dialog->setSuffixes(headerSuffix(), sourceSuffix(), formSuffix());
    dialog->setBaseClasses(baseClasses());
    return dialog;
}

Core::GeneratedFiles OSXAppWizard::generateFiles(const QWizard *w,
                                                 QString *errorMessage) const
{
    const OSXAppWizardDialog *dialog = qobject_cast<const OSXAppWizardDialog *>(w);
    const QtProjectParameters projectParams = dialog->projectParameters();
    const QString projectPath = projectParams.projectPath();
    const GuiAppParameters params = dialog->parameters();
    const QString license = CppTools::AbstractEditorSupport::licenseTemplate();

    // Generate file names. Note that the path for the project files is the
    // newly generated project directory.
    const QString templatePath = osxTemplateDir();
    // Create files: main source
    QString contents;
    const QString mainSourceFileName = buildFileName(projectPath, QLatin1String(mainSourceFileC), sourceSuffix());
    Core::GeneratedFile mainSource(mainSourceFileName);
    if (!parametrizeTemplate(templatePath, QLatin1String("main.cpp"), params, &contents, errorMessage))
        return Core::GeneratedFiles();
    mainSource.setContents(CppTools::AbstractEditorSupport::licenseTemplate(mainSourceFileName)
                           + contents);
    // Change the formSource and formHeader to our code.

    // Copy our "template" files to the destination folder, start with .mm.
    // Due to how QtWizard operates and that this isn't a wizard w/ xml config,
    // we need to create the directory before the QtWizard handles template deployment.
    QDir templPath = QDir(templatePath);
    if (!templPath.exists())
        qErrnoWarning("Template Path " + templPath.absolutePath().toLatin1() + " does not exist.");
    // Create dir using Source-based path
    const QString cocoaTargetSource = buildFileName(projectPath, QLatin1String(cocoaC), objcppSuffix());
    const QString cocoaTargetHeader = buildFileName(projectPath, QLatin1String(cocoaC), headerSuffix());
    const QString mwinTargetSource = buildFileName(projectPath, QLatin1String(mainWindowFileC), sourceSuffix());
    const QString mwinTargetHeader = buildFileName(projectPath, QLatin1String(mainWindowFileC), headerSuffix());
    const QString mwinTargetForm = buildFileName(projectPath, QLatin1String(mainWindowFileC), formSuffix());

    const QString objcSourceFile = templPath.absoluteFilePath(QFileInfo(cocoaTargetSource).fileName());
    const QString objcHeaderFile = templPath.absoluteFilePath(QFileInfo(cocoaTargetHeader).fileName());
    const QString mwinHeaderFile = templPath.absoluteFilePath(QFileInfo(mwinTargetHeader).fileName());
    const QString mwinSourceFile = templPath.absoluteFilePath(QFileInfo(mwinTargetSource).fileName());
    const QString mwinFormFile = templPath.absoluteFilePath(QFileInfo(mwinTargetForm).fileName());

    // Copy files
    QDir targetDir = QFileInfo(cocoaTargetSource).absoluteDir();
    targetDir.mkpath(targetDir.absolutePath());

    QFile::copy(objcSourceFile, cocoaTargetSource);
    QFile::copy(objcHeaderFile, cocoaTargetHeader);
    QFile::copy(mwinHeaderFile, mwinTargetHeader);
    QFile::copy(mwinSourceFile, mwinTargetSource);
    QFile::copy(mwinFormFile, mwinTargetForm);

    // Create files: profile
    const QString profileName = buildFileName(projectPath, projectParams.fileName, profileSuffix());
    Core::GeneratedFile profile(profileName);
    profile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);
    contents.clear();
    {
        QTextStream proStr(&contents);
        QtProjectParameters::writeProFileHeader(proStr);
        projectParams.writeProFile(proStr);
        proStr << "\n\nSOURCES += " << QFileInfo(mainSourceFileName).fileName()
               << "\\\n        mainwindow.cpp"
               << "\n\nHEADERS  += mainwindow.h"
               << "\n\nmacx { "
               << "\n\n    LIBS += -framework Cocoa"
               << "\n\n    QT += macextras"
               << "\n\n    OBJECTIVE_SOURCES += cocoainitializer.mm"
               << "\n    OBJECTIVE_HEADERS += cocoainitializer.h"
               << "\n\n}\n"
               << "\n\nFORMS    += mainwindow.ui\n";
    }
    profile.setContents(contents);
    // List
    Core::GeneratedFiles rc;
    rc << mainSource << profile;
    return rc;
}

bool OSXAppWizard::parametrizeTemplate(const QString &templatePath, const QString &templateName,
                                       const GuiAppParameters &params,
                                       QString *target, QString *errorMessage)
{
    QString fileName = templatePath;
    fileName += QDir::separator();
    fileName += templateName;
    Utils::FileReader reader;
    if (!reader.fetch(fileName, QIODevice::Text, errorMessage))
        return false;
    QString contents = QString::fromUtf8(reader.data());
    contents.replace(QLatin1String("%QAPP_INCLUDE%"), QLatin1String("QApplication"));
    contents.replace(QLatin1String("%INCLUDE%"), params.headerFileName);
    contents.replace(QLatin1String("%CLASS%"), params.className);
    contents.replace(QLatin1String("%BASECLASS%"), params.baseClassName);
    contents.replace(QLatin1String("%WIDGET_HEIGHT%"), QString::number(params.widgetHeight));
    contents.replace(QLatin1String("%WIDGET_WIDTH%"), QString::number(params.widgetWidth));
    if (params.isMobileApplication)
        contents.replace(QLatin1String("%SHOWMETHOD%"), QString::fromLatin1(mainSourceMobilityShowC));
    else
        contents.replace(QLatin1String("%SHOWMETHOD%"), QString::fromLatin1(mainSourceShowC));


    const QChar dot = QLatin1Char('.');

    QString preDef = params.headerFileName.toUpper();
    preDef.replace(dot, QLatin1Char('_'));
    contents.replace(QLatin1String("%PRE_DEF%"), preDef);

    const QString uiFileName = params.formFileName;
    QString uiHdr = QLatin1String("ui_");
    uiHdr += uiFileName.left(uiFileName.indexOf(dot));
    uiHdr += QLatin1String(".h");

    contents.replace(QLatin1String("%UI_HDR%"), uiHdr);
    if (params.baseClassName == QLatin1String("QMainWindow")) {
        if (params.isMobileApplication)
            contents.replace(QLatin1String("%CENTRAL_WIDGET%"), QLatin1String(mainWindowMobileUiContentsC));
        else
            contents.replace(QLatin1String("%CENTRAL_WIDGET%"), QLatin1String(mainWindowUiContentsC));
    } else {
        contents.remove(QLatin1String("%CENTRAL_WIDGET%"));
    }
    *target = contents;
    return true;
}

} // namespace Internal
} // namespace QmakeProjectManager
