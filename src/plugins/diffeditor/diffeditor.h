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

#ifndef DIFFEDITOR_H
#define DIFFEDITOR_H

#include "diffeditor_global.h"
#include "diffeditorcontroller.h"

#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/idocument.h>

QT_BEGIN_NAMESPACE
class QToolBar;
class QComboBox;
class QToolButton;
QT_END_NAMESPACE

namespace TextEditor { class BaseTextEditorWidget; }

namespace DiffEditor {

class DiffEditorDocument;
class DiffEditorGuiController;
class SideBySideDiffEditorWidget;

class DIFFEDITOR_EXPORT DiffEditor : public Core::IEditor
{
    Q_OBJECT
public:
    DiffEditor();
    DiffEditor(DiffEditor *other);
    virtual ~DiffEditor();

public:
    DiffEditorController *controller() const;

    // Core::IEditor
    bool duplicateSupported() const { return false; }
    Core::IEditor *duplicate();

    bool open(QString *errorString, const QString &fileName, const QString &realFileName);
    Core::IDocument *document();

    QWidget *toolBar();

public slots:
    void activateEntry(int index);

private slots:
    void slotCleared(const QString &message);
    void slotDiffContentsChanged(const QList<DiffEditorController::DiffFilesContents> &diffFileList,
                                 const QString &workingDirectory);
    void entryActivated(int index);
    void slotDescriptionChanged(const QString &description);
    void slotDescriptionVisibilityChanged();

private:
    void ctor();
    void updateEntryToolTip();

    QSharedPointer<DiffEditorDocument> m_document;
    TextEditor::BaseTextEditorWidget *m_descriptionWidget;
    SideBySideDiffEditorWidget *m_diffWidget;
    DiffEditorController *m_controller;
    DiffEditorGuiController *m_guiController;
    QToolBar *m_toolBar;
    QComboBox *m_entriesComboBox;
    QAction *m_toggleDescriptionAction;
};

} // namespace DiffEditor

#endif // DIFFEDITOR_H
