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

#ifndef QMLJSEDITORDOCUMENT_P_H
#define QMLJSEDITORDOCUMENT_P_H

#include <qmljs/qmljsdocument.h>
#include <qmljstools/qmljssemanticinfo.h>

#include <QObject>
#include <QTextLayout>
#include <QTimer>

namespace QmlJSEditor {

class QmlJSEditorDocument;

namespace Internal {

class QmlOutlineModel;
class SemanticHighlighter;
class SemanticInfoUpdater;

class QmlJSEditorDocumentPrivate : public QObject
{
    Q_OBJECT

public:
    QmlJSEditorDocumentPrivate(QmlJSEditorDocument *parent);
    ~QmlJSEditorDocumentPrivate();

public slots:
    void invalidateFormatterCache();
    void reparseDocument();
    void onDocumentUpdated(QmlJS::Document::Ptr doc);
    void reupdateSemanticInfo();
    void acceptNewSemanticInfo(const QmlJSTools::SemanticInfo &semanticInfo);
    void updateOutlineModel();

public:
    QmlJSEditorDocument *q;
    QTimer m_updateDocumentTimer; // used to compress multiple document changes
    QTimer m_reupdateSemanticInfoTimer; // used to compress multiple libraryInfo changes
    int m_semanticInfoDocRevision; // document revision to which the semantic info is currently updated to
    SemanticInfoUpdater *m_semanticInfoUpdater;
    QmlJSTools::SemanticInfo m_semanticInfo;
    QVector<QTextLayout::FormatRange> m_diagnosticRanges;
    Internal::SemanticHighlighter *m_semanticHighlighter;
    bool m_semanticHighlightingNecessary;
    bool m_outlineModelNeedsUpdate;
    QTimer m_updateOutlineModelTimer;
    Internal::QmlOutlineModel *m_outlineModel;
};

} // Internal
} // QmlJSEditor

#endif // QMLJSEDITORDOCUMENT_P_H
