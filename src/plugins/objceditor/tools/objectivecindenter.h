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

#ifndef OBJECTIVECINDENTER_H
#define OBJECTIVECINDENTER_H

#include <texteditor/indenter.h>
#include <QStringList>

namespace ObjectiveCEditor {

class ObjectiveCIndenter : public TextEditor::Indenter
{
public:
    ObjectiveCIndenter();
    virtual ~ObjectiveCIndenter();

    bool isElectricCharacter(const QChar &ch) const;
    void indentBlock(QTextDocument *document,
                     const QTextBlock &block,
                     const QChar &typedChar,
                     const TextEditor::TabSettings &settings);

protected:
    bool isElectricLine(const QString &line) const;
    int getIndentDiff(const QString &previousLine) const;

private:
    QStringList m_jumpKeywords;
};

} // namespace ObjectiveCEditor

#endif // OBJECTIVECINDENTER_H
