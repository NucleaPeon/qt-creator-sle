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

#include "designeractionmanagerview.h"

#include <selectioncontext.h>
#include <abstractdesigneraction.h>
#include "tabviewdesigneraction.h"

namespace QmlDesigner {

DesignerActionManagerView::DesignerActionManagerView()
    : AbstractView(0),
      m_designerActionManager(this),
      m_isInRewriterTransaction(false),
      m_setupContextDirty(false)
{
    m_designerActionManager.createDefaultDesignerActions();

    m_designerActionManager.addDesignerAction(new TabViewDesignerAction);
}

void DesignerActionManagerView::modelAttached(Model *model)
{
    AbstractView::modelAttached(model);
    setupContext();
}

void DesignerActionManagerView::modelAboutToBeDetached(Model *model)
{
    AbstractView::modelAboutToBeDetached(model);
    setupContext();
}

void DesignerActionManagerView::nodeCreated(const ModelNode &)
{
    setupContext();
}

void DesignerActionManagerView::nodeAboutToBeRemoved(const ModelNode &)
{}

void DesignerActionManagerView::nodeRemoved(const ModelNode &, const NodeAbstractProperty &, AbstractView::PropertyChangeFlags)
{
    setupContext();
}

void DesignerActionManagerView::nodeAboutToBeReparented(const ModelNode &, const NodeAbstractProperty &, const NodeAbstractProperty &, AbstractView::PropertyChangeFlags)
{
    setupContext();
}

void DesignerActionManagerView::nodeReparented(const ModelNode &, const NodeAbstractProperty &, const NodeAbstractProperty &, AbstractView::PropertyChangeFlags)
{
    setupContext();
}

void DesignerActionManagerView::nodeIdChanged(const ModelNode &, const QString &, const QString &)
{}

void DesignerActionManagerView::propertiesAboutToBeRemoved(const QList<AbstractProperty> &)
{}

void DesignerActionManagerView::propertiesRemoved(const QList<AbstractProperty> &)
{
    setupContext();
}

void DesignerActionManagerView::variantPropertiesChanged(const QList<VariantProperty> &, AbstractView::PropertyChangeFlags)
{}

void DesignerActionManagerView::bindingPropertiesChanged(const QList<BindingProperty> &, AbstractView::PropertyChangeFlags)
{}

void DesignerActionManagerView::rootNodeTypeChanged(const QString &, int, int)
{
    setupContext();
}

void DesignerActionManagerView::instancePropertyChange(const QList<QPair<ModelNode, PropertyName> > &)
{}

void DesignerActionManagerView::instancesCompleted(const QVector<ModelNode> &)
{}

void DesignerActionManagerView::instanceInformationsChange(const QMultiHash<ModelNode, InformationName> &)
{}

void DesignerActionManagerView::instancesRenderImageChanged(const QVector<ModelNode> &)
{}

void DesignerActionManagerView::instancesPreviewImageChanged(const QVector<ModelNode> &)
{}

void DesignerActionManagerView::instancesChildrenChanged(const QVector<ModelNode> &)
{}

void DesignerActionManagerView::instancesToken(const QString &, int, const QVector<ModelNode> &)
{}

void DesignerActionManagerView::nodeSourceChanged(const ModelNode &, const QString &)
{}

void DesignerActionManagerView::rewriterBeginTransaction()
{
    m_isInRewriterTransaction = true;
}

void DesignerActionManagerView::rewriterEndTransaction()
{
    m_isInRewriterTransaction = false;

    if (m_setupContextDirty)
        setupContext();
}

void DesignerActionManagerView::currentStateChanged(const ModelNode &)
{
    setupContext();
}

void DesignerActionManagerView::selectedNodesChanged(const QList<ModelNode> &, const QList<ModelNode> &)
{
    setupContext();
}

void DesignerActionManagerView::nodeOrderChanged(const NodeListProperty &, const ModelNode &, int)
{
    setupContext();
}

void DesignerActionManagerView::importsChanged(const QList<Import> &, const QList<Import> &)
{
    setupContext();
}

void DesignerActionManagerView::scriptFunctionsChanged(const ModelNode &, const QStringList &)
{}

void DesignerActionManagerView::setDesignerActionList(const QList<AbstractDesignerAction *> &designerActionList)
{
    m_designerActionList = designerActionList;
}

void DesignerActionManagerView::signalHandlerPropertiesChanged(const QVector<SignalHandlerProperty> &, AbstractView::PropertyChangeFlags)
{
    setupContext();
}

DesignerActionManager &DesignerActionManagerView::designerActionManager()
{
    return m_designerActionManager;
}

const DesignerActionManager &DesignerActionManagerView::designerActionManager() const
{
    return m_designerActionManager;
}

void DesignerActionManagerView::setupContext()
{
    if (m_isInRewriterTransaction) {
        m_setupContextDirty = true;
        return;
    }
    SelectionContext selectionContext(this);
    foreach (AbstractDesignerAction* action, m_designerActionList) {
        action->currentContextChanged(selectionContext);
    }
    m_setupContextDirty = false;
}


} // namespace QmlDesigner
