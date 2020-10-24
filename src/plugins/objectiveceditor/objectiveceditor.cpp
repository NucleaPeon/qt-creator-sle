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

/**
  \class PyEditor::Editor implements interface Core::IEditor
  This editor makes possible to edit ObjectiveC source files
  */

#include "objectiveceditor.h"
#include "objectiveceditorconstants.h"
#include "objectiveceditorplugin.h"
#include "objectiveceditorwidget.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>

#include <QFileInfo>

namespace ObjectiveCEditor {
namespace Internal {

ObjectiveCEditor::ObjectiveCEditor(EditorWidget *editorWidget)
    :BaseTextEditor(editorWidget)
{
    setId(Constants::C_OBJECTIVEC_EDITOR_ID);
    setContext(Core::Context(Constants::C_OBJECTIVEC_EDITOR_ID,
                             TextEditor::Constants::C_TEXTEDITOR));
}

ObjectiveCEditor::~ObjectiveCEditor()
{
}

Core::IEditor *ObjectiveCEditor::duplicate()
{
    EditorWidget *widget = new EditorWidget(qobject_cast<EditorWidget *>(editorWidget()));
    TextEditor::TextEditorSettings::initializeEditor(widget);
    return widget->editor();
}

bool ObjectiveCEditor::open(QString *errorString,
                        const QString &fileName,
                        const QString &realFileName)
{
    Core::MimeType mimeType = Core::MimeDatabase::findByFile(QFileInfo(fileName));
    baseTextDocument()->setMimeType(mimeType.type());
    return TextEditor::BaseTextEditor::open(errorString, fileName, realFileName);
}

} // namespace Internal
} // namespace ObjectiveCEditor
