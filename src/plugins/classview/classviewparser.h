/**************************************************************************
**
** Copyright (c) 2014 Denis Mingulov
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

#ifndef CLASSVIEWPARSER_H
#define CLASSVIEWPARSER_H

#include <QObject>

#include "classviewparsertreeitem.h"

#include <cplusplus/CPlusPlusForwardDeclarations.h>
#include <cplusplus/CppDocument.h>

// might be changed to forward declaration - is not done to be less dependent
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/project.h>

#include <QList>
#include <QSharedPointer>
#include <QStandardItem>

namespace ClassView {
namespace Internal {

class ParserPrivate;

class Parser : public QObject
{
    Q_OBJECT

public:
    explicit Parser(QObject *parent = 0);
    ~Parser();

    bool canFetchMore(QStandardItem *item) const;

    void fetchMore(QStandardItem *item, bool skipRoot = false) const;

signals:
    //! File list is changed
    void filesAreRemoved();

    void treeDataUpdate(QSharedPointer<QStandardItem> result);

    void resetDataDone();

public slots:
    void clearCacheAll();

    void clearCache();

    void requestCurrentState();

    void setFileList(const QStringList &fileList);

    void removeFiles(const QStringList &fileList);

    void resetData(const CPlusPlus::Snapshot &snapshot);

    void resetDataToCurrentState();

    void parseDocument(const CPlusPlus::Document::Ptr &doc);

    void setFlatMode(bool flat);

protected slots:
    void onResetDataDone();

protected:
    void addProject(const ParserTreeItem::Ptr &item, const QStringList &fileList,
                    const QString &projectId = QString());

    void addSymbol(const ParserTreeItem::Ptr &item, const CPlusPlus::Symbol *symbol);

    ParserTreeItem::ConstPtr getParseDocumentTree(const CPlusPlus::Document::Ptr &doc);

    ParserTreeItem::ConstPtr getCachedOrParseDocumentTree(const CPlusPlus::Document::Ptr &doc);

    ParserTreeItem::Ptr getParseProjectTree(const QStringList &fileList, const QString &projectId);

    ParserTreeItem::Ptr getCachedOrParseProjectTree(const QStringList &fileList,
                                                    const QString &projectId);

    void emitCurrentTree();

    ParserTreeItem::ConstPtr parse();

    ParserTreeItem::ConstPtr findItemByRoot(const QStandardItem *item, bool skipRoot = false) const;

    QStringList addProjectNode(const ParserTreeItem::Ptr &item,
                               const ProjectExplorer::ProjectNode *node);

    QStringList projectNodeFileList(const ProjectExplorer::FolderNode *node) const;

    ParserTreeItem::Ptr createFlatTree(const QStringList &projectList);

private:
    //! Private class data pointer
    ParserPrivate *d;
};

} // namespace Internal
} // namespace ClassView

#endif // CLASSVIEWPARSER_H
