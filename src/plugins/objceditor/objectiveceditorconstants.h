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

#ifndef OBJECTIVECEDITOR_CONSTANTS_H
#define OBJECTIVECEDITOR_CONSTANTS_H

#include <QtGlobal>

namespace ObjectiveCEditor {
namespace Constants {

const char C_OBJECTIVECEDITOR_ID[] = "ObjectiveCEditor.ObjectiveCEditor";
const char C_EDITOR_DISPLAY_NAME[] =
        QT_TRANSLATE_NOOP("OpenWith::Editors", "ObjectiveC Editor");

/*******************************************************************************
 * File creation wizard
 ******************************************************************************/
const char C_OBJC_WIZARD_CATEGORY[] = "U.ObjectiveC";
const char C_OBJC_EXTENSION[] = ".py";
const char C_OBJC_DISPLAY_CATEGORY[] = "ObjectiveC";

    // source
const char C_OBJC_SOURCE_WIZARD_ID[] = "P.ObjCSource";
const char C_OBJC_SOURCE_CONTENT[] = "";
//         "#!/usr/bin/env python\n"
//         "# -*- coding: utf-8 -*-\n"
//         "\n";
const char EN_OBJC_SOURCE_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("ObjectiveCEditor::FileWizard", "ObjectiveC source file");
const char EN_OBJC_SOURCE_DESCRIPTION[] =
        QT_TRANSLATE_NOOP("ObjectiveCEditor::FileWizard", "Creates an empty ObjectiveC source file");

    // class
const char C_OBJC_CLASS_WIZARD_ID[] = "P.ObjCClass";
const char EN_OBJC_CLASS_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("ObjectiveCEditor::ClassWizard", "ObjectiveC class");
const char EN_OBJC_CLASS_DESCRIPTION[] =
        QT_TRANSLATE_NOOP("ObjectiveCEditor::ClassWizard", "Creates new ObjectiveC class");

/*******************************************************************************
 * MIME type
 ******************************************************************************/
const char OBJC_SOURCE_MIMETYPE[] = "text/x-objcsrc";
const char OBJCPP_SOURCE_MIMETYPE[] = "text/x-objc++src";
const char CPP_HEADER_MIMETYPE[] = "text/x-c++hdr";
const char C_HEADER_MIMETYPE[] = "text/x-chdr";

} // namespace Constants
} // namespace ObjectiveCEditor

#endif // PYTHONEDITOR_CONSTANTS_H
