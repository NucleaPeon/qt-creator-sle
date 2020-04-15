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

#ifndef CLASSVIEWPARSERTREEITEM_H
#define CLASSVIEWPARSERTREEITEM_H

#include "classviewsymbollocation.h"
#include "classviewsymbolinformation.h"

#include <QSharedPointer>

QT_FORWARD_DECLARE_CLASS(QStandardItem)

namespace ClassView {
namespace Internal {

class ParserTreeItemPrivate;

class ParserTreeItem
{
public:
    typedef QSharedPointer<ParserTreeItem> Ptr;
    typedef QSharedPointer<const ParserTreeItem> ConstPtr;

public:
    ParserTreeItem();
    ~ParserTreeItem();

    void copyTree(const ParserTreeItem::ConstPtr &from);

    void copy(const ParserTreeItem::ConstPtr &from);

    void addSymbolLocation(const SymbolLocation &location);

    void addSymbolLocation(const QSet<SymbolLocation> &locations);

    void removeSymbolLocation(const SymbolLocation &location);

    void removeSymbolLocations(const QSet<SymbolLocation> &locations);

    QSet<SymbolLocation> symbolLocations() const;

    void appendChild(const ParserTreeItem::Ptr &item, const SymbolInformation &inf);

    void removeChild(const SymbolInformation &inf);

    ParserTreeItem::Ptr child(const SymbolInformation &inf) const;

    int childCount() const;

    void convertTo(QStandardItem *item, bool recursive = true) const;

    // additional properties
    //! Assigned icon
    QIcon icon() const;

    //! Set an icon for this tree node
    void setIcon(const QIcon &icon);

    void add(const ParserTreeItem::ConstPtr &target);

    void subtract(const ParserTreeItem::ConstPtr &target);

    bool canFetchMore(QStandardItem *item) const;

    void fetchMore(QStandardItem *item) const;

    void debugDump(int ident = 0) const;

protected:
    ParserTreeItem &operator=(const ParserTreeItem &other);

private:
    //! Private class data pointer
    ParserTreeItemPrivate *d;
};

} // namespace Internal
} // namespace ClassView

#endif // CLASSVIEWPARSERTREEITEM_H
