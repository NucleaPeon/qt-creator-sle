/**************************************************************************
**
** Copyright (c) 2014 Lorenz Haas
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

#include "clangformat.h"

#include "clangformatconstants.h"
#include "clangformatoptionspage.h"
#include "clangformatsettings.h"

#include "../beautifierconstants.h"
#include "../beautifierplugin.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>
#include <cppeditor/cppeditorconstants.h>
#include <texteditor/basetexteditor.h>

#include <QAction>
#include <QMenu>

namespace Beautifier {
namespace Internal {
namespace ClangFormat {

ClangFormat::ClangFormat(QObject *parent) :
    BeautifierAbstractTool(parent),
    m_settings(new ClangFormatSettings)
{
}

ClangFormat::~ClangFormat()
{
    delete m_settings;
}

bool ClangFormat::initialize()
{
    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::ClangFormat::MENU_ID);
    menu->menu()->setTitle(QLatin1String("ClangFormat"));

    m_formatFile = new QAction(BeautifierPlugin::msgFormatCurrentFile(), this);
    Core::Command *cmd
            = Core::ActionManager::registerAction(m_formatFile,
                                                  Constants::ClangFormat::ACTION_FORMATFILE,
                                                  Core::Context(Core::Constants::C_GLOBAL));
    menu->addAction(cmd);
    connect(m_formatFile, SIGNAL(triggered()), this, SLOT(formatFile()));

    m_formatRange = new QAction(BeautifierPlugin::msgFormatSelectedText(), this);
    cmd = Core::ActionManager::registerAction(m_formatRange,
                                              Constants::ClangFormat::ACTION_FORMATSELECTED,
                                              Core::Context(Core::Constants::C_GLOBAL));
    menu->addAction(cmd);
    connect(m_formatRange, SIGNAL(triggered()), this, SLOT(formatSelectedText()));

    Core::ActionManager::actionContainer(Constants::MENU_ID)->addMenu(menu);

    return true;
}

void ClangFormat::updateActions(Core::IEditor *editor)
{
    const bool enabled = (editor && editor->id() == CppEditor::Constants::CPPEDITOR_ID);
    m_formatFile->setEnabled(enabled);
    m_formatRange->setEnabled(enabled);
}

QList<QObject *> ClangFormat::autoReleaseObjects()
{
    ClangFormatOptionsPage *optionsPage = new ClangFormatOptionsPage(m_settings, this);
    return QList<QObject *>() << optionsPage;
}

void ClangFormat::formatFile()
{
    BeautifierPlugin::formatCurrentFile(command());
}

void ClangFormat::formatSelectedText()
{
    TextEditor::BaseTextEditor *editor
            = qobject_cast<TextEditor::BaseTextEditor *>(Core::EditorManager::currentEditor());
    if (!editor)
        return;

    QTextCursor tc = editor->editorWidget()->textCursor();
    if (tc.hasSelection()) {
        const int offset = tc.selectionStart();
        const int length = tc.selectionEnd() - offset;
        BeautifierPlugin::formatCurrentFile(command(offset, length));
    } else if (m_settings->formatEntireFileFallback()) {
        formatFile();
    }
}

QStringList ClangFormat::command(int offset, int length) const
{
    QStringList command;
    command << m_settings->command();
    command << QLatin1String("-i");
    if (m_settings->usePredefinedStyle()) {
        command << QLatin1String("-style=") + m_settings->predefinedStyle();
    } else {
        command << QLatin1String("-style={") + m_settings->style(
                       m_settings->customStyle()).remove(QLatin1Char('\n')) + QLatin1String("}");
    }
    if (offset != -1) {
        command << QLatin1String("-offset=") + QString::number(offset);
        command << QLatin1String("-length=") + QString::number(length);
    }
    command << QLatin1String("%file");

    return command;
}

} // namespace ClangFormat
} // namespace Internal
} // namespace Beautifier
