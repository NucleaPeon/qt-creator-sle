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

#ifndef CPPTOOLS_CPPCOMPLETIONASSISTPROVIDER_H
#define CPPTOOLS_CPPCOMPLETIONASSISTPROVIDER_H

#include "cpptools_global.h"

#include <texteditor/codeassist/assistenums.h>
#include <texteditor/codeassist/completionassistprovider.h>

#include <utils/qtcoverride.h>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace ProjectExplorer { class Project; }

namespace TextEditor {
class BaseTextEditor;
class IAssistInterface;
}

namespace CppTools {

class CPPTOOLS_EXPORT CppCompletionAssistProvider : public TextEditor::CompletionAssistProvider
{
    Q_OBJECT

public:
    bool supportsEditor(const Core::Id &editorId) const QTC_OVERRIDE;
    int activationCharSequenceLength() const QTC_OVERRIDE;
    bool isActivationCharSequence(const QString &sequence) const QTC_OVERRIDE;

    virtual TextEditor::IAssistInterface *createAssistInterface(
            ProjectExplorer::Project *project, TextEditor::BaseTextEditor *editor,
            QTextDocument *document, int position, TextEditor::AssistReason reason) const = 0;

    static int activationSequenceChar(const QChar &ch, const QChar &ch2,
                                      const QChar &ch3, unsigned *kind,
                                      bool wantFunctionCall);
};

} // namespace CppTools

#endif // CPPTOOLS_CPPCOMPLETIONASSISTPROVIDER_H
