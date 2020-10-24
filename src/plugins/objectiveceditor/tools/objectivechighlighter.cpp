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
 * @brief The Highlighter class pre-highlights ObjectiveC source using simple scanner.
 *
 * Highlighter doesn't highlight user types (classes and enumerations), syntax
 * and semantic errors, unnecessary code, etc. It's implements only
 * basic highlight mechanism.
 *
 * Main highlight procedure is highlightBlock().
 */

#include "objectivechighlighter.h"
#include "lexical/objectivecscanner.h"

#include <texteditor/basetextdocument.h>
#include <texteditor/texteditorconstants.h>

namespace ObjectiveCEditor {

using namespace ObjectiveCEditor::Internal;

/**
 * @class ObjCEditor::Highlighter
 * @brief Handles incremental lexical highlighting, but not semantic
 *
 * Incremental lexical highlighting works every time when any character typed
 * or some text inserted (i.e. copied & pasted).
 * Each line keeps associated scanner state - integer number. This state is the
 * scanner context for next line. For example, 3 quotes begin a multiline
 * string, and each line up to next 3 quotes has state 'MultiLineString'.
 *
 * @code
 *  def __init__:               # Normal
 *      self.__doc__ = """      # MultiLineString (next line is inside)
 *                     banana   # MultiLineString
 *                     """      # Normal
 * @endcode
 */

ObjectiveCHighlighter::ObjectiveCHighlighter(QTextDocument *parent) :
    TextEditor::SyntaxHighlighter(parent)
{
    init();
}

/// New instance created when opening any document in editor
ObjectiveCHighlighter::ObjectiveCHighlighter(TextEditor::BaseTextDocument *parent) :
    TextEditor::SyntaxHighlighter(parent)
{
    init();
}

void ObjectiveCHighlighter::init()
{
    static QVector<TextEditor::TextStyle> categories;
    if (categories.isEmpty()) {
        categories << TextEditor::C_NUMBER
                   << TextEditor::C_STRING
                   << TextEditor::C_TYPE
                   << TextEditor::C_KEYWORD
                   << TextEditor::C_OPERATOR
                   << TextEditor::C_PREPROCESSOR
                   << TextEditor::C_LABEL
                   << TextEditor::C_COMMENT
                   << TextEditor::C_DOXYGEN_COMMENT
                   << TextEditor::C_DOXYGEN_TAG
                   << TextEditor::C_VISUAL_WHITESPACE;
    }
    setTextFormatCategories(categories);
}

/// Instance destroyed when one of documents closed from editor
ObjectiveCHighlighter::~ObjectiveCHighlighter()
{
}

/**
 * @brief Highlighter::highlightBlock highlights single line of ObjectiveC code
 * @param text is single line without EOLN symbol. Access to all block data
 * can be obtained through inherited currentBlock() function.
 *
 * This function receives state (int number) from previously highlighted block,
 * scans block using received state and sets initial highlighting for current
 * block. At the end, it saves internal state in current block.
 */
void ObjectiveCHighlighter::highlightBlock(const QString &text)
{
    int initialState = previousBlockState();
    if (initialState == -1)
        initialState = 0;
    setCurrentBlockState(highlightLine(text, initialState));
}

/**
 * @return True if this keyword is acceptable at start of import line
 */
static inline
bool isImportKeyword(const QString &keyword)
{
    return (keyword == QLatin1String("#include"));
}

/**
 * @brief Highlight line of code, returns new block state
 * @param text Source code to highlight
 * @param initialState Initial state of scanner, retrieved from previous block
 * @return Final state of scanner, should be saved with current block
 */
int ObjectiveCHighlighter::highlightLine(const QString &text, int initialState)
{
    Scanner scanner(text.constData(), text.size());
    scanner.setState(initialState);

    FormatToken tk;
    bool hasOnlyWhitespace = true;
    while ((tk = scanner.read()).format() != Format_EndOfBlock) {
        Format format = tk.format();
        if (format == Format_Keyword) {
            QString value = scanner.value(tk);
            if (isImportKeyword(value) && hasOnlyWhitespace) {
                setFormat(tk.begin(), tk.length(), formatForCategory(format));
                highlightImport(scanner);
                break;
            }
        }

        setFormat(tk.begin(), tk.length(), formatForCategory(format));
        if (format != Format_Whitespace)
            hasOnlyWhitespace = false;
    }
    return scanner.state();
}

/**
 * @brief Highlights rest of line as import directive
 */
void ObjectiveCHighlighter::highlightImport(Scanner &scanner)
{
    FormatToken tk;
    while ((tk = scanner.read()).format() != Format_EndOfBlock) {
        Format format = tk.format();
        if (tk.format() == Format_Identifier)
            format = Format_ImportedModule;
        setFormat(tk.begin(), tk.length(), formatForCategory(format));
    }
}

} // namespace ObjectiveCEditor
