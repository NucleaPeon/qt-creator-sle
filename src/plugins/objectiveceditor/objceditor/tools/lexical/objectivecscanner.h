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

#ifndef OBJECTIVECSCANNER_H
#define OBJECTIVECSCANNER_H

#include "objectivecformattoken.h"
#include "sourcecodestream.h"

#include <QString>
#include <QSet>

namespace ObjectiveCEditor {
namespace Internal {

/**
 * @brief The Scanner class - scans source code for highlighting only
 */
class Scanner
{
    Scanner(const Scanner &other);
    void operator =(const Scanner &other);

public:
    enum State {
        State_Default,
        State_String,
        State_MultiLineString
    };

    Scanner(const QChar *text, const int length);
    ~Scanner();

    void setState(int state);
    int state() const;
    FormatToken read();
    QString value(const FormatToken& tk) const;

private:
    FormatToken onDefaultState();

    void checkEscapeSequence(QChar quoteChar);
    FormatToken readStringLiteral(QChar quoteChar);
    FormatToken readMultiLineStringLiteral(QChar quoteChar);
    FormatToken readIdentifier();
    FormatToken readNumber();
    FormatToken readFloatNumber();
    FormatToken readComment();
    FormatToken readDoxygenComment();
    FormatToken readWhiteSpace();
    FormatToken readOperator();

    void clearState();
    void saveState(State state, QChar savedData);
    void parseState(State &state, QChar &savedData) const;

    SourceCodeStream m_src;
    int m_state;
    const QSet<QString> m_keywords;
    const QSet<QString> m_magics;
    const QSet<QString> m_builtins;
};

} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVECSCANNER_H
