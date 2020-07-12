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

#ifndef OBJECTIVECFORMATTOKEN_H
#define OBJECTIVECFORMATTOKEN_H

#include <stdlib.h>

namespace ObjectiveCEditor {
namespace Internal {

enum Format {
    Format_Number = 0,
    Format_String,
    Format_Keyword,
    Format_Type,
    Format_ClassField,
    Format_MagicAttr, // magic class attribute/method, like __name__, __init__
    Format_Operator,
    Format_Comment,
    Format_Doxygen,
    Format_Identifier,
    Format_Whitespace,
    Format_ImportedModule,

    Format_FormatsAmount,
    Format_EndOfBlock
};

class FormatToken
{
public:
    FormatToken() {}

    FormatToken(Format format, size_t position, size_t length)
        :m_format(format)
        ,m_position(position)
        ,m_length(length)
    {}

    inline Format format() const { return m_format; }
    inline int begin() const { return m_position; }
    inline int end() const { return m_position + m_length; }
    inline int length() const { return m_length; }

private:
    Format m_format;
    int m_position;
    int m_length;
};

} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVECFORMATTOKEN_H
