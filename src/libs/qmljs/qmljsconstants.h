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

#ifndef QMLJSCONSTANTS_H
#define QMLJSCONSTANTS_H

namespace QmlJS {

namespace ImportType {
enum Enum {
    Invalid,
    Library,
    Directory,
    ImplicitDirectory,
    File,
    UnknownFile, // refers a file/directory that wasn't found (or to an url)
    QrcDirectory,
    QrcFile
};
}

namespace ImportKind {
enum Enum {
    Invalid,
    Library,
    Path,
    QrcPath
};
}

namespace Severity {
enum Enum
{
    Hint,         // cosmetic or convention
    MaybeWarning, // possibly a warning, insufficient information
    Warning,      // could cause unintended behavior
    MaybeError,   // possibly an error, insufficient information
    Error         // definitely an error
};
}

namespace Language {
enum Enum
{
    Unknown = 0,
    JavaScript = 1,
    Json = 2,
    Qml = 3,
    QmlQtQuick1 = 4,
    QmlQtQuick2 = 5,
    QmlQbs = 6,
    QmlProject = 7,
    QmlTypeInfo = 8
};
}

namespace Constants {

const char TASK_INDEX[] = "QmlJSEditor.TaskIndex";
const char TASK_IMPORT_SCAN[] = "QmlJSEditor.TaskImportScan";

} // namespace Constants
} // namespace QmlJS
#endif // QMLJSCONSTANTS_H
