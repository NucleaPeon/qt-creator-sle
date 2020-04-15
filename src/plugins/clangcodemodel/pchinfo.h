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

#ifndef PCHINFO_H
#define PCHINFO_H

#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QTemporaryFile>

namespace ClangCodeModel {
namespace Internal {

class PchInfo
{
    PchInfo();

public:
    typedef QSharedPointer<PchInfo> Ptr;

public:
    ~PchInfo();

    static Ptr createEmpty();
    static Ptr createWithFileName(const QString &inputFileName,
                                  const QStringList &options, bool objcEnabled);

    /// \return the (temporary) file name for the PCH file.
    QString fileName() const
    { return m_file.fileName(); }

    /// \return the input file for the PCH compilation.
    QString inputFileName() const
    { return m_inputFileName; }

    /// \return the options used to generate this PCH file.
    QStringList options() const
    { return m_options; }

    bool objcWasEnabled() const
    { return m_objcEnabled; }

private:
    QString m_inputFileName;
    QStringList m_options;
    bool m_objcEnabled;
    QTemporaryFile m_file;
};

} // Internal namespace
} // ClangCodeModel namespace

#endif // PCHINFO_H
