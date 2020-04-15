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

#ifndef QMLDESIGNER_DESIGNERACTIONMANAGERVIEW_H
#define QMLDESIGNER_DESIGNERACTIONMANAGERVIEW_H

#include <abstractview.h>

#include "designeractionmanager.h"

namespace QmlDesigner {

class AbstractDesignerAction;

class DesignerActionManagerView : public AbstractView
{
    Q_OBJECT
public:
    DesignerActionManagerView();

    void modelAttached(Model *model);
    void modelAboutToBeDetached(Model *model);
    void nodeCreated(const ModelNode &);
    void nodeAboutToBeRemoved(const ModelNode &);
    void nodeRemoved(const ModelNode &, const NodeAbstractProperty &, PropertyChangeFlags);
    void nodeAboutToBeReparented(const ModelNode &,
                                         const NodeAbstractProperty &,
                                         const NodeAbstractProperty &,
                                         AbstractView::PropertyChangeFlags );
    void nodeReparented(const ModelNode &, const NodeAbstractProperty &,
                                const NodeAbstractProperty &,
                                AbstractView::PropertyChangeFlags);
    void nodeIdChanged(const ModelNode&, const QString&, const QString&);
    void propertiesAboutToBeRemoved(const QList<AbstractProperty>&);
    void propertiesRemoved(const QList<AbstractProperty>&);
    void variantPropertiesChanged(const QList<VariantProperty>&, PropertyChangeFlags);
    void bindingPropertiesChanged(const QList<BindingProperty>&, PropertyChangeFlags);
    void rootNodeTypeChanged(const QString &, int , int );
    void instancePropertyChange(const QList<QPair<ModelNode, PropertyName> > &);
    void instancesCompleted(const QVector<ModelNode> &);
    void instanceInformationsChange(const QMultiHash<ModelNode, InformationName> &);
    void instancesRenderImageChanged(const QVector<ModelNode> &);
    void instancesPreviewImageChanged(const QVector<ModelNode> &);
    void instancesChildrenChanged(const QVector<ModelNode> &);
    void instancesToken(const QString &, int , const QVector<ModelNode> &);
    void nodeSourceChanged(const ModelNode &, const QString &);
    void rewriterBeginTransaction();
    void rewriterEndTransaction();
    void currentStateChanged(const ModelNode &);
    void selectedNodesChanged(const QList<ModelNode> &,
                                      const QList<ModelNode> &);
    void nodeOrderChanged(const NodeListProperty &, const ModelNode &, int );
    void importsChanged(const QList<Import> &, const QList<Import> &);
    void scriptFunctionsChanged(const ModelNode &, const QStringList &);
    void setDesignerActionList(const QList<AbstractDesignerAction* > &designerActionList);
    void signalHandlerPropertiesChanged(const QVector<SignalHandlerProperty> &/*propertyList*/, PropertyChangeFlags /*propertyChange*/);

    DesignerActionManager &designerActionManager();
    const DesignerActionManager &designerActionManager() const;

protected:
    void setupContext();

private:
    DesignerActionManager m_designerActionManager;
    QList<AbstractDesignerAction* > m_designerActionList;
    bool m_isInRewriterTransaction;
    bool m_setupContextDirty;
};


} // namespace QmlDesigner

#endif // QMLDESIGNER_DESIGNERACTIONMANAGERVIEW_H
