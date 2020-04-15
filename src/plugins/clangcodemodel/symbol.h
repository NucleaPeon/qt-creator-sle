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

#ifndef INDEXEDSYMBOLINFO_H
#define INDEXEDSYMBOLINFO_H

#include "sourcelocation.h"

#include <QString>
#include <QDataStream>
#include <QIcon>

namespace ClangCodeModel {

class Symbol
{
public:
    enum Kind {
        Enum,
        Class,
        Method,       // A member-function.
        Function,     // A free-function (global or within a namespace).
        Declaration,
        Constructor,
        Destructor,
        Unknown
    };

    Symbol();
    Symbol(const QString &name,
           const QString &qualification,
           Kind type,
           const SourceLocation &location);

    QString m_name;
    QString m_qualification;
    SourceLocation m_location;
    Kind m_kind;

    QIcon iconForSymbol() const;
};

QDataStream &operator<<(QDataStream &stream, const Symbol &symbol);
QDataStream &operator>>(QDataStream &stream, Symbol &symbol);

bool operator==(const Symbol &a, const Symbol &b);
bool operator!=(const Symbol &a, const Symbol &b);

} // Clang

#endif // INDEXEDSYMBOLINFO_H
