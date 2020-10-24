/****************************************************************************
**
** Copyright (C) 2020 PeonDevelopments 
** Contact: Daniel Kettle <initial.dann@gmail.com>
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
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

#ifndef OBJECTIVECEDITOR_EDITOR_H
#define OBJECTIVECEDITOR_EDITOR_H

#include <texteditor/basetexteditor.h>

namespace ObjectiveCEditor {
namespace Internal {

class EditorWidget;

class ObjectiveCEditor : public TextEditor::BaseTextEditor
{
    Q_OBJECT

public:
    explicit ObjectiveCEditor(EditorWidget *editorWidget);
    virtual ~ObjectiveCEditor();

    bool duplicateSupported() const { return true; }
    Core::IEditor *duplicate();

    /**
      Opens file for editing, actual work performed by base class
      */
    bool open(QString *errorString,
              const QString &fileName,
              const QString &realFileName);
};

} // namespace Internal
} // namespace ObjectiveCEditor

#endif // OBJECTIVECEDITOR_EDITOR_H
