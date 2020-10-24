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
  \class EditorWidget
  Graphical representation and parent widget for __Editor::Editor class.
  Represents editor as plain text editor.
  */

#include "objectiveceditorwidget.h"
#include "tools/objectivechighlighter.h"
#include "tools/objectivecindenter.h"
#include "objectiveceditor.h"

#include <texteditor/fontsettings.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/basetextdocument.h>
#include <texteditor/indenter.h>
#include <texteditor/autocompleter.h>

namespace ObjectiveCEditor {
namespace Internal {

EditorWidget::EditorWidget(QWidget *parent)
    : TextEditor::BaseTextEditorWidget(parent)
{
    baseTextDocument()->setIndenter(new ObjectiveCIndenter());
    ctor();
}

EditorWidget::EditorWidget(EditorWidget *other)
    : TextEditor::BaseTextEditorWidget(other)
{
    ctor();
}

void EditorWidget::ctor()
{
    m_commentDefinition.multiLineStart.clear();
    m_commentDefinition.multiLineEnd.clear();
    m_commentDefinition.singleLine = QLatin1Char('//');

    setParenthesesMatchingEnabled(true);
    setMarksVisible(true);
    setCodeFoldingSupported(true);

    new ObjectiveCHighlighter(baseTextDocument());
}

EditorWidget::~EditorWidget()
{
}

/**
  Comments or uncomments selection using Python commenting syntax
  */
void EditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this, m_commentDefinition);
}

TextEditor::BaseTextEditor *EditorWidget::createEditor()
{
    return new ObjectiveCEditor(this);
}

} // namespace Internal
} // namespace ObjectiveCEditor
