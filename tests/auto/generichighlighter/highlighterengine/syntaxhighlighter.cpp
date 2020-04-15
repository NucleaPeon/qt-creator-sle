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

#include "syntaxhighlighter.h"
#include "formats.h"

#include <texteditor/generichighlighter/highlighter.h>

using namespace TextEditor;
using namespace Internal;

QTextCharFormat SyntaxHighlighter::formatForCategory(int categoryIndex) const
{
    switch (categoryIndex) {
    case Highlighter::Keyword:      return Formats::instance().keywordFormat();
    case Highlighter::DataType:     return Formats::instance().dataTypeFormat();
    case Highlighter::Decimal:      return Formats::instance().decimalFormat();
    case Highlighter::BaseN:        return Formats::instance().baseNFormat();
    case Highlighter::Float:        return Formats::instance().floatFormat();
    case Highlighter::Char:         return Formats::instance().charFormat();
    case Highlighter::String:       return Formats::instance().stringFormat();
    case Highlighter::Comment:      return Formats::instance().commentFormat();
    case Highlighter::Alert:        return Formats::instance().alertFormat();
    case Highlighter::Error:        return Formats::instance().errorFormat();
    case Highlighter::Function:     return Formats::instance().functionFormat();
    case Highlighter::RegionMarker: return Formats::instance().regionMarketFormat();
    case Highlighter::Others:       return Formats::instance().othersFormat();
    default:                                  return QTextCharFormat();
    }
}

