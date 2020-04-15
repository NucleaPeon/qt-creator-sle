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

#ifndef RESOURCEEDITOR_CONSTANTS_H
#define RESOURCEEDITOR_CONSTANTS_H

namespace ResourceEditor {
namespace Constants {

const char C_RESOURCEEDITOR[] = "Qt4.ResourceEditor";
const char RESOURCEEDITOR_ID[] = "Qt4.ResourceEditor";
const char C_RESOURCEEDITOR_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("OpenWith::Editors", "Resource Editor");

const char REFRESH[] = "ResourceEditor.Refresh";

const char C_RESOURCE_MIMETYPE[] = "application/vnd.qt.xml.resource";

const char C_ADD_PREFIX[] = "ResourceEditor.AddPrefix";
const char C_REMOVE_PREFIX[] = "ResourceEditor.RemovePrefix";
const char C_RENAME_PREFIX[] = "ResourceEditor.RenamePrefix";

const char C_REMOVE_FILE[] = "ResourceEditor.RemoveFile";
const char C_RENAME_FILE[] = "ResourceEditor.RenameFile";

const char C_OPEN_EDITOR[] = "ResourceEditor.OpenEditor";
const char C_OPEN_TEXT_EDITOR[] = "ResourceEditor.OpenTextEditor";


} // namespace Constants
} // namespace ResourceEditor

#endif // RESOURCEEDITOR_CONSTANTS_H
