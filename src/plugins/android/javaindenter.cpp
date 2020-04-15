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

#include "javaindenter.h"
#include <texteditor/tabsettings.h>

#include <QTextDocument>

using namespace Android;
using namespace Android::Internal;

JavaIndenter::JavaIndenter()
{
}

JavaIndenter::~JavaIndenter()
{}

bool JavaIndenter::isElectricCharacter(const QChar &ch) const
{
    if (ch == QLatin1Char('{')
            || ch == QLatin1Char('}')) {
        return true;
    }
    return false;
}

void JavaIndenter::indentBlock(QTextDocument *doc,
                                 const QTextBlock &block,
                                 const QChar &typedChar,
                                 const TextEditor::TabSettings &tabSettings)
{
    // At beginning: Leave as is.
    if (block == doc->begin())
        return;

    const int tabsize = tabSettings.m_indentSize;

    QTextBlock previous = block.previous();
    QString previousText = previous.text();
    while (previousText.trimmed().isEmpty()) {
        previous = previous.previous();
        if (previous == doc->begin())
            return;
        previousText = previous.text();
    }

    int adjust = 0;
    if (previousText.contains(QLatin1Char('{')))
        adjust = tabsize;

    if (block.text().contains(QLatin1Char('}')) || typedChar == QLatin1Char('}'))
        adjust += -tabsize;

    // Count the indentation of the previous line.
    int i = 0;
    while (i < previousText.size()) {
        if (!previousText.at(i).isSpace()) {
            tabSettings.indentLine(block, tabSettings.columnAt(previousText, i)
                                   + adjust);
            break;
        }
        ++i;
    }
}
