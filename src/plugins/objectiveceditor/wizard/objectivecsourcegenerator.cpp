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

#include "objectivecsourcegenerator.h"
#include <QSet>

namespace ObjectiveCEditor {
namespace Internal {

static const char BASH_RUN_HEADER[] = "#!/usr/bin/env objc-run\n# You must set up objc-run to enable this functionality.";
static const char ENCODING_HEADER[] = "# See https://github.com/iljaiwas/objc-run\n";
static const char FOUNDATION_HEADER[] = "#import <Foundation/Foundation.h>";
static const char MAIN_HEADER[] = "int main(int argc, const char * argv[])";

SourceGenerator::SourceGenerator()
{
}

SourceGenerator::~SourceGenerator()
{
}

QString SourceGenerator::generateClass(const QString &className,
                                       Utils::NewClassWidget::ClassType classType) const
{
    // Objective C itself has no class functionality, only structs.
    QString ret;
    ret.reserve(1024);
    ret += QLatin1String(FOUNDATION_HEADER); // FIXME: Create a comment or something here
    ret += QLatin1Char('\n');
    return ret;
}

/**
* @brief Generates main file of command line application in Objective C
*
* Class MainWindow should be defined in 'mainwindow' module.
* @param windowTitle Title for created window instance
*/
QString SourceGenerator::generateObjectiveCMain(const QString &windowTitle) const
{
    QString ret;
    ret.reserve(1024);
    ret += QLatin1String(FOUNDATION_HEADER);
    ret += QLatin1Char('\n');
    ret += QLatin1Char('\n');
    ret += QLatin1String(MAIN_HEADER);
    ret += QLatin1Char('\n');
    ret += QLatin1Char('{');
    ret += QLatin1Char('\n');
    ret += QLatin1Char('\t');
    ret += QLatin1String("return 0;");
    ret += QLatin1Char('\n');
    ret += QLatin1Char('}');

    return ret;
}

/**
* @brief Generates main file of command line application in Objective CPP
*
* Class MainWindow should be defined in 'mainwindow' module.
* @param windowTitle Title for created window instance
*/
QString SourceGenerator::generateObjectiveCPPMain(const QString &windowTitle) const
{
    QString ret;
    ret.reserve(1024);
    ret += QLatin1String(FOUNDATION_HEADER);
    ret += QLatin1Char('\n');
    ret += QLatin1Char('\n');
    ret += QLatin1String(MAIN_HEADER);
    ret += QLatin1Char('\n');
    ret += QLatin1Char('{');
    ret += QLatin1Char('\n');
    ret += QLatin1String("\t@autoreleasepool {\n");
    ret += QLatin1String("\t\tNSLog(@\"Hello, World!\");\n");
    ret += QLatin1String("\t}");
    ret += QLatin1String("\treturn 0;");
    ret += QLatin1Char('\n');
    ret += QLatin1Char('}');

    return ret;
}

QString SourceGenerator::qtModulesImport(const QSet<QString> &modules) const
{
//    QString slotsImport;
//    if (modules.contains(QLatin1String("QtCore")))
//        slotsImport = QLatin1String("    from PyQt4.QtCore import pyqtSlot as Slot\n");

//    QLatin1String defaultBinding("PySide");
//    QLatin1String fallbackBinding("PyQt4");
//    if (m_objectivecQtBinding == PyQt)
//        qSwap(defaultBinding, fallbackBinding);

    QString ret;
    ret.reserve(256);
//    ret += QLatin1String("try:\n");
//    if (m_objectivecQtBinding == PyQt)
//        ret += slotsImport;
//    foreach (const QString &name, modules)
//        ret += QString::fromLatin1("    from %1 import %2\n")
//                .arg(defaultBinding)
//                .arg(name);

//    ret += QLatin1String("except:\n");
//    if (m_objectivecQtBinding != PyQt)
//        ret += slotsImport;
//    foreach (const QString &name, modules)
//        ret += QString::fromLatin1("    from %1 import %2\n")
//                .arg(fallbackBinding)
//                .arg(name);

    return ret;
}

QString SourceGenerator::moduleForQWidget() const
{

}

} // namespace Internal
} // namespace ObjectiveCEditor
