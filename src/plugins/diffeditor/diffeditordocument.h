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

#ifndef DIFFEDITORDOCUMENT_H
#define DIFFEDITORDOCUMENT_H

#include "diffeditor_global.h"

#include <coreplugin/idocument.h>

namespace DiffEditor {

class DiffEditorController;

class DIFFEDITOR_EXPORT DiffEditorDocument : public Core::IDocument
{
    Q_OBJECT
public:
    explicit DiffEditorDocument();
    virtual ~DiffEditorDocument();

    DiffEditorController *controller() const;

    bool setContents(const QByteArray &contents);
    QString defaultPath() const { return QString(); }
    QString suggestedFileName() const { return QString(); }

    bool isModified() const { return false; }
    QString mimeType() const;
    bool isSaveAsAllowed() const { return false; }
    bool save(QString *errorString, const QString &fileName, bool autoSave);
    ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
    bool reload(QString *errorString, ReloadFlag flag, ChangeType type);

private:
    DiffEditorController *m_diffEditorController;
};

} // namespace DiffEditor

#endif // DIFFEDITORDOCUMENT_H
