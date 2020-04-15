/****************************************************************************
**
** Copyright (C) 2014 Przemyslaw Gorszkowski <pgorszkowski@gmail.com>
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

#include "cppincludehierarchy.h"

#include "cppeditor.h"
#include "cppeditorconstants.h"
#include "cppeditorplugin.h"
#include "cppelementevaluator.h"
#include "cppincludehierarchymodel.h"
#include "cppincludehierarchytreeview.h"

#include <coreplugin/editormanager/editormanager.h>
#include <cplusplus/CppDocument.h>

#include <utils/annotateditemdelegate.h>
#include <utils/fileutils.h>

#include <QDir>
#include <QLabel>
#include <QLatin1String>
#include <QModelIndex>
#include <QStandardItem>
#include <QVBoxLayout>

using namespace CppEditor;
using namespace CppEditor::Internal;
using namespace Utils;

namespace CppEditor {
namespace Internal {

class CppIncludeLabel : public QLabel
{
public:
    CppIncludeLabel(QWidget *parent)
        : QLabel(parent)
    {}

    void setup(const QString &fileName, const QString &filePath)
    {
        setText(fileName);
        m_link = CPPEditorWidget::Link(filePath);
    }

private:
    void mousePressEvent(QMouseEvent *)
    {
        if (!m_link.hasValidTarget())
            return;

        Core::EditorManager::openEditorAt(m_link.targetFileName,
                                          m_link.targetLine,
                                          m_link.targetColumn,
                                          Constants::CPPEDITOR_ID);
    }

    CPPEditorWidget::Link m_link;
};

// CppIncludeHierarchyWidget
CppIncludeHierarchyWidget::CppIncludeHierarchyWidget() :
    QWidget(0),
    m_treeView(0),
    m_model(0),
    m_delegate(0),
    m_includeHierarchyInfoLabel(0),
    m_editor(0)
{
    m_inspectedFile = new CppIncludeLabel(this);
    m_inspectedFile->setMargin(5);
    m_model = new CppIncludeHierarchyModel(this);
    m_treeView = new CppIncludeHierarchyTreeView(this);
    m_delegate = new AnnotatedItemDelegate(this);
    m_delegate->setDelimiter(QLatin1String(" "));
    m_delegate->setAnnotationRole(AnnotationRole);
    m_treeView->setModel(m_model);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->setItemDelegate(m_delegate);

    connect(m_treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));

    m_includeHierarchyInfoLabel = new QLabel(tr("No include hierarchy available"), this);
    m_includeHierarchyInfoLabel->setAlignment(Qt::AlignCenter);
    m_includeHierarchyInfoLabel->setAutoFillBackground(true);
    m_includeHierarchyInfoLabel->setBackgroundRole(QPalette::Base);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_inspectedFile);
    layout->addWidget(m_treeView);
    layout->addWidget(m_includeHierarchyInfoLabel);
    setLayout(layout);

    connect(CppEditorPlugin::instance(), SIGNAL(includeHierarchyRequested()), SLOT(perform()));
    connect(Core::EditorManager::instance(), SIGNAL(editorsClosed(QList<Core::IEditor*>)),
            this, SLOT(editorsClosed(QList<Core::IEditor*>)));

}

CppIncludeHierarchyWidget::~CppIncludeHierarchyWidget()
{
}

void CppIncludeHierarchyWidget::perform()
{
    showNoIncludeHierarchyLabel();

    m_editor = qobject_cast<CPPEditor *>(Core::EditorManager::currentEditor());
    if (!m_editor)
        return;

    CPPEditorWidget *widget = qobject_cast<CPPEditorWidget *>(m_editor->widget());
    if (!widget)
        return;

    m_model->clear();
    m_model->buildHierarchy(m_editor, widget->baseTextDocument()->filePath());
    if (m_model->isEmpty())
        return;

    m_inspectedFile->setup(widget->baseTextDocument()->displayName(),
                           widget->baseTextDocument()->filePath());

    //expand "Includes"
    m_treeView->expand(m_model->index(0, 0));
    //expand "Included by"
    m_treeView->expand(m_model->index(1, 0));

    showIncludeHierarchy();
}

void CppIncludeHierarchyWidget::onItemClicked(const QModelIndex &index)
{
    const TextEditor::BaseTextEditorWidget::Link link
            = index.data(LinkRole).value<TextEditor::BaseTextEditorWidget::Link>();
    if (link.hasValidTarget())
        Core::EditorManager::openEditorAt(link.targetFileName,
                                          link.targetLine,
                                          link.targetColumn,
                                          Constants::CPPEDITOR_ID);
}

void CppIncludeHierarchyWidget::editorsClosed(QList<Core::IEditor *> editors)
{
    foreach (Core::IEditor *editor, editors) {
        if (m_editor == editor)
            perform();
    }
}

void CppIncludeHierarchyWidget::showNoIncludeHierarchyLabel()
{
    m_inspectedFile->hide();
    m_treeView->hide();
    m_includeHierarchyInfoLabel->show();
}

void CppIncludeHierarchyWidget::showIncludeHierarchy()
{
    m_inspectedFile->show();
    m_treeView->show();
    m_includeHierarchyInfoLabel->hide();
}

// CppIncludeHierarchyStackedWidget
CppIncludeHierarchyStackedWidget::CppIncludeHierarchyStackedWidget(QWidget *parent) :
    QStackedWidget(parent),
    m_typeHiearchyWidgetInstance(new CppIncludeHierarchyWidget)
{
    addWidget(m_typeHiearchyWidgetInstance);
}

CppIncludeHierarchyStackedWidget::~CppIncludeHierarchyStackedWidget()
{
    delete m_typeHiearchyWidgetInstance;
}

// CppIncludeHierarchyFactory
CppIncludeHierarchyFactory::CppIncludeHierarchyFactory()
{
}

CppIncludeHierarchyFactory::~CppIncludeHierarchyFactory()
{
}

QString CppIncludeHierarchyFactory::displayName() const
{
    return tr("Include Hierarchy");
}

int CppIncludeHierarchyFactory::priority() const
{
    return Constants::INCLUDE_HIERARCHY_PRIORITY;
}

Core::Id CppIncludeHierarchyFactory::id() const
{
    return Core::Id(Constants::INCLUDE_HIERARCHY_ID);
}

QKeySequence CppIncludeHierarchyFactory::activationSequence() const
{
    return QKeySequence();
}

Core::NavigationView CppIncludeHierarchyFactory::createWidget()
{
    CppIncludeHierarchyStackedWidget *w = new CppIncludeHierarchyStackedWidget;
    static_cast<CppIncludeHierarchyWidget *>(w->currentWidget())->perform();
    Core::NavigationView navigationView;
    navigationView.widget = w;
    return navigationView;
}

} // namespace Internal
} // namespace CppEditor
