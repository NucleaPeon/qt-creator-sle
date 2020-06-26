/****************************************************************************
**
** Copyright (C) 2020 PeonDevelopments 
** Contact: Daniel Kettle <initial.dann@gmail.com>
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

#include "objectiveceditorfactory.h"
#include "objectivecditorconstants.h"
#include "objectiveceditorwidget.h"
#include "objectiveceditorplugin.h"

#include <coreplugin/icore.h>
#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorsettings.h>

#include <QDebug>

namespace ObjectiveCEditor {
namespace Internal {

EditorFactory::EditorFactory(QObject *parent)
    : Core::IEditorFactory(parent)
{
    setId(Constants::C_OBJECTIVECEDITOR_ID);
    setDisplayName(tr(Constants::C_EDITOR_DISPLAY_NAME));
    addMimeType(QLatin1String(Constants::C_OBJC_IMETYPE));
    new TextEditor::TextEditorActionHandler(this,
                              Constants::C_OBJECTIVECEDITOR_ID,
                              TextEditor::TextEditorActionHandler::Format
                              | TextEditor::TextEditorActionHandler::UnCommentSelection
                              | TextEditor::TextEditorActionHandler::UnCollapseAll);
}

Core::IEditor *EditorFactory::createEditor()
{
    EditorWidget *widget = new EditorWidget();
    TextEditor::TextEditorSettings::initializeEditor(widget);

    return widget->editor();
}

} // namespace Internal
} // namespace ObjectiveCEditor
