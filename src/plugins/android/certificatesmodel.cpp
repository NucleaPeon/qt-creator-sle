/****************************************************************************
**
** Copyright (c) 2014 BogDan Vatra <bog_dan_ro@yahoo.com>
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

#include "certificatesmodel.h"

#include <QComboBox>

using namespace Android;
using namespace Android::Internal;

namespace {
const QLatin1String AliasString("Alias name:");
const QLatin1String CertificateSeparator("*******************************************");
}

CertificatesModel::CertificatesModel(const QString &rowCertificates, QObject *parent)
    : QAbstractListModel(parent)
{
    int from = rowCertificates.indexOf(AliasString);
    QPair<QString, QString> item;
    while (from > -1) {
        from += 11;// strlen(AliasString);
        const int eol = rowCertificates.indexOf(QLatin1Char('\n'), from);
        item.first = rowCertificates.mid(from, eol - from).trimmed();
        const int eoc = rowCertificates.indexOf(CertificateSeparator, eol);
        item.second = rowCertificates.mid(eol + 1, eoc - eol - 2).trimmed();
        from = rowCertificates.indexOf(AliasString, eoc);
        m_certs.push_back(item);
    }
}

int CertificatesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_certs.size();
}

QVariant CertificatesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::ToolTipRole))
        return QVariant();
    if (role == Qt::DisplayRole)
        return m_certs[index.row()].first;
    return m_certs[index.row()].second;
}
