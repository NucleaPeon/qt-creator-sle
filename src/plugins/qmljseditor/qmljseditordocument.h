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

#ifndef QMLJSEDITORDOCUMENT_H
#define QMLJSEDITORDOCUMENT_H

#include "qmljseditor_global.h"

#include <qmljs/qmljsdocument.h>
#include <qmljstools/qmljssemanticinfo.h>
#include <texteditor/basetextdocument.h>

#include <QTextLayout>

namespace QmlJSEditor {

namespace Internal {
class QmlJSEditorDocumentPrivate;
class QmlOutlineModel;
} // Internal

class QMLJSEDITOR_EXPORT QmlJSEditorDocument : public TextEditor::BaseTextDocument
{
    Q_OBJECT
public:
    QmlJSEditorDocument();
    ~QmlJSEditorDocument();

    const QmlJSTools::SemanticInfo &semanticInfo() const;
    bool isSemanticInfoOutdated() const;
    QVector<QTextLayout::FormatRange> diagnosticRanges() const;
    void setDiagnosticRanges(const QVector<QTextLayout::FormatRange> &ranges);
    Internal::QmlOutlineModel *outlineModel() const;

signals:
    void updateCodeWarnings(QmlJS::Document::Ptr doc);
    void semanticInfoUpdated(const QmlJSTools::SemanticInfo &semanticInfo);

protected:
    void applyFontSettings();
    void triggerPendingUpdates();

private:
    friend class Internal::QmlJSEditorDocumentPrivate; // sending signals
    Internal::QmlJSEditorDocumentPrivate *d;
};

} // QmlJSEditor

#endif // QMLJSEDITORDOCUMENT_H
