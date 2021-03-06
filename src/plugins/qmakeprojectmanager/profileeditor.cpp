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

#include "profileeditor.h"

#include "profilehighlighter.h"
#include "qmakeprojectmanagerconstants.h"
#include "profileeditorfactory.h"
#include "profilecompletionassist.h"

#include <extensionsystem/pluginmanager.h>

#include <texteditor/fontsettings.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorsettings.h>

#include <QFileInfo>
#include <QDir>
#include <QSharedPointer>
#include <QTextBlock>

namespace QmakeProjectManager {
namespace Internal {

//
// ProFileEditor
//

ProFileEditor::ProFileEditor(ProFileEditorWidget *editor)
  : BaseTextEditor(editor)
{
    setId(Constants::PROFILE_EDITOR_ID);
    setContext(Core::Context(Constants::C_PROFILEEDITOR,
              TextEditor::Constants::C_TEXTEDITOR));
}

Core::IEditor *ProFileEditor::duplicate()
{
    ProFileEditorWidget *ret = new ProFileEditorWidget(
                qobject_cast<ProFileEditorWidget*>(editorWidget()));
    TextEditor::TextEditorSettings::initializeEditor(ret);
    return ret->editor();
}

TextEditor::CompletionAssistProvider *ProFileEditor::completionAssistProvider()
{
    return ExtensionSystem::PluginManager::getObject<ProFileCompletionAssistProvider>();
}

//
// ProFileEditorWidget
//

ProFileEditorWidget::ProFileEditorWidget(QWidget *parent)
    : BaseTextEditorWidget(new ProFileDocument(), parent)
{
    ctor();
}

ProFileEditorWidget::ProFileEditorWidget(ProFileEditorWidget *other)
    : BaseTextEditorWidget(other)
{
    ctor();
}

void ProFileEditorWidget::ctor()
{
    m_commentDefinition.clearCommentStyles();
    m_commentDefinition.singleLine = QLatin1Char('#');
}

void ProFileEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this, m_commentDefinition);
}

static bool isValidFileNameChar(const QChar &c)
{
    return c.isLetterOrNumber()
            || c == QLatin1Char('.')
            || c == QLatin1Char('_')
            || c == QLatin1Char('-')
            || c == QLatin1Char('/')
            || c == QLatin1Char('\\');
}

ProFileEditorWidget::Link ProFileEditorWidget::findLinkAt(const QTextCursor &cursor,
                                                          bool /*resolveTarget*/,
                                                          bool /*inNextSplit*/)
{
    Link link;

    int lineNumber = 0, positionInBlock = 0;
    convertPosition(cursor.position(), &lineNumber, &positionInBlock);

    const QString block = cursor.block().text();

    // check if the current position is commented out
    const int hashPos = block.indexOf(QLatin1Char('#'));
    if (hashPos >= 0 && hashPos < positionInBlock)
        return link;

    // find the beginning of a filename
    QString buffer;
    int beginPos = positionInBlock - 1;
    while (beginPos >= 0) {
        QChar c = block.at(beginPos);
        if (isValidFileNameChar(c)) {
            buffer.prepend(c);
            beginPos--;
        } else {
            break;
        }
    }

    // find the end of a filename
    int endPos = positionInBlock;
    while (endPos < block.count()) {
        QChar c = block.at(endPos);
        if (isValidFileNameChar(c)) {
            buffer.append(c);
            endPos++;
        } else {
            break;
        }
    }

    if (buffer.isEmpty())
        return link;

    // remove trailing '\' since it can be line continuation char
    if (buffer.at(buffer.size() - 1) == QLatin1Char('\\')) {
        buffer.chop(1);
        endPos--;
    }

    // if the buffer starts with $$PWD accept it
    if (buffer.startsWith(QLatin1String("PWD/")) ||
            buffer.startsWith(QLatin1String("PWD\\"))) {
        if (beginPos > 0 && block.mid(beginPos - 1, 2) == QLatin1String("$$")) {
            beginPos -=2;
            buffer = buffer.mid(4);
        }
    }

    QDir dir(QFileInfo(baseTextDocument()->filePath()).absolutePath());
    QString fileName = dir.filePath(buffer);
    QFileInfo fi(fileName);
    if (fi.exists()) {
        if (fi.isDir()) {
            QDir subDir(fi.absoluteFilePath());
            QString subProject = subDir.filePath(subDir.dirName() + QLatin1String(".pro"));
            if (QFileInfo(subProject).exists())
                fileName = subProject;
            else
                return link;
        }
        link.targetFileName = QDir::cleanPath(fileName);
        link.linkTextStart = cursor.position() - positionInBlock + beginPos + 1;
        link.linkTextEnd = cursor.position() - positionInBlock + endPos;
    }
    return link;
}

TextEditor::BaseTextEditor *ProFileEditorWidget::createEditor()
{
    return new ProFileEditor(this);
}

void ProFileEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    showDefaultContextMenu(e, Constants::M_CONTEXT);
}

//
// ProFileDocument
//

ProFileDocument::ProFileDocument()
        : TextEditor::BaseTextDocument()
{
    setMimeType(QLatin1String(Constants::PROFILE_MIMETYPE));
    setSyntaxHighlighter(new ProFileHighlighter);
}

QString ProFileDocument::defaultPath() const
{
    QFileInfo fi(filePath());
    return fi.absolutePath();
}

QString ProFileDocument::suggestedFileName() const
{
    QFileInfo fi(filePath());
    return fi.fileName();
}

} // namespace Internal
} // namespace QmakeProjectManager
