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

#ifndef OBJECTIVECHIGHLIGHTER_H
#define OBJECTIVECHIGHLIGHTER_H

#include <texteditor/syntaxhighlighter.h>

namespace ObjectiveCEditor {

namespace Internal { class Scanner; }

class ObjectiveCHighlighter : public TextEditor::SyntaxHighlighter
{
    Q_OBJECT
public:
    explicit ObjectiveCHighlighter(QTextDocument *parent = 0);
    explicit ObjectiveCHighlighter(TextEditor::BaseTextDocument *parent);
    virtual ~ObjectiveCHighlighter();

protected:
    virtual void highlightBlock(const QString &text);

private:
    int highlightLine(const QString &text, int initialState);
    void highlightImport(Internal::Scanner &scanner);
    void init();
};

} // namespace ObjectiveCEditor

#endif // OBJECTIVECHIGHLIGHTER_H
