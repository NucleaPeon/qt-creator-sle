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

#ifndef CLANGCODEMODEL_COMPLETIONPROPOSALSBUILDER_H
#define CLANGCODEMODEL_COMPLETIONPROPOSALSBUILDER_H

#include "clangcompleter.h"
#include "clang_global.h"
#include <clang-c/Index.h>

#include <QCoreApplication>

namespace ClangCodeModel {

class CLANG_EXPORT CompletionProposalsBuilder
{
    Q_DECLARE_TR_FUNCTIONS(ClangCodeModel::CompletionProposalsBuilder)

public:
    CompletionProposalsBuilder(QList<CodeCompletionResult> &results, quint64 contexts, bool isSignalSlotCompletion);
    void operator ()(const CXCompletionResult &cxResult);

private:
    struct OptionalChunk {
        int positionInHint;
        QString hint;

        OptionalChunk() : positionInHint(0) {}
    };

    static CodeCompletionResult::Kind getKind(const CXCompletionResult &cxResult);
    static CodeCompletionResult::Availability getAvailability(const CXCompletionResult &cxResult);
    static int findNameInPlaceholder(const QString &text);
    void resetWithResult(const CXCompletionResult &cxResult);
    void finalize();

    void concatChunksForObjectiveCMessage(const CXCompletionResult &cxResult);
    void concatChunksForNestedName(const CXCompletionString &cxString);
    void concatChunksAsSnippet(const CXCompletionString &cxString);
    void concatChunksOnlyTypedText(const CXCompletionString &cxString);
    void concatSlotSignalSignature(const CXCompletionString &cxString);
    void appendOptionalChunks(const CXCompletionString &cxString,
                              int insertionIndex);
    void attachResultTypeToComment(const QString &text);

    void appendSnippet(const QString &text);
    void appendHintBold(const QString &text);

    QList<CodeCompletionResult> &m_results;
    const quint64 m_contexts;
    const bool m_isSignalSlotCompletion;

    unsigned m_priority;
    CodeCompletionResult::Kind m_resultKind;
    CodeCompletionResult::Availability m_resultAvailability;
    bool m_hasParameters;
    QString m_hint;
    QString m_text;
    QString m_snippet;
    QString m_comment;
    QList<OptionalChunk> m_optionalChunks;
};

} // namespace ClangCodeModel

#endif // CLANGCODEMODEL_COMPLETIONPROPOSALSBUILDER_H
