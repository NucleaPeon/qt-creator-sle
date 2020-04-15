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


#ifndef CPPFOLLOWSYMBOLUNDERCURSOR_H
#define CPPFOLLOWSYMBOLUNDERCURSOR_H

#include <cplusplus/CppDocument.h>
#include <texteditor/basetexteditor.h>

QT_BEGIN_NAMESPACE
class QTextCursor;
QT_END_NAMESPACE

namespace CppTools { class SymbolFinder; }

namespace CppEditor {
namespace Internal {

class CPPEditorWidget;
class VirtualFunctionAssistProvider;

class FollowSymbolUnderCursor
{
public:
    typedef TextEditor::BaseTextEditorWidget::Link Link;

    FollowSymbolUnderCursor(CPPEditorWidget *widget);
    ~FollowSymbolUnderCursor();

    Link findLink(const QTextCursor &cursor, bool resolveTarget,
                  const CPlusPlus::Snapshot &snapshot,
                  const CPlusPlus::Document::Ptr &documentFromSemanticInfo,
                  CppTools::SymbolFinder *symbolFinder, bool inNextSplit);

    VirtualFunctionAssistProvider *virtualFunctionAssistProvider();
    void setVirtualFunctionAssistProvider(VirtualFunctionAssistProvider *provider);

private:
    CPPEditorWidget *m_widget;
    VirtualFunctionAssistProvider *m_virtualFunctionAssistProvider;
};

} // namespace Internal
} // namespace CppEditor

#endif // CPPFOLLOWSYMBOLUNDERCURSOR_H
