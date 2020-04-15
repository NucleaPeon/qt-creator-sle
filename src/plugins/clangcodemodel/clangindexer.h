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

#ifndef CLANGINDEXER_H
#define CLANGINDEXER_H

#include "fastindexer.h"

#include <cpptools/cppindexingsupport.h>

#include <QObject>

namespace ClangCodeModel {

class Indexer;

namespace Internal {

class ClangIndexer;
class ClangSymbolSearcher;

class ClangIndexingSupport: public CppTools::CppIndexingSupport
{
public:
    ClangIndexingSupport(ClangIndexer *indexer);
    virtual ~ClangIndexingSupport();

    virtual QFuture<void> refreshSourceFiles(const QStringList &sourceFiles);
    virtual CppTools::SymbolSearcher *createSymbolSearcher(CppTools::SymbolSearcher::Parameters parameters, QSet<QString> fileNames);

private:
    ClangIndexer *m_indexer;
};

class ClangIndexer: public QObject, public FastIndexer
{
    Q_OBJECT

public:
    ClangIndexer();
    ~ClangIndexer();

    CppTools::CppIndexingSupport *indexingSupport();

    QFuture<void> refreshSourceFiles(const QStringList &sourceFiles);

    void match(ClangSymbolSearcher *searcher) const;

    void indexNow(Unit::Ptr unit);

public slots:
    void onAboutToLoadSession(const QString &sessionName);
    void onSessionLoaded(QString);
    void onAboutToSaveSession();

private slots:
    void onIndexingStarted(QFuture<void> indexingFuture);

private:
    QScopedPointer<ClangIndexingSupport> m_indexingSupport;
    bool m_isLoadingSession;
    Indexer *m_clangIndexer;
};

} // namespace Internal
} // namespace ClangCodeModel

#endif // CLANGINDEXER_H
