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

#ifndef STATESEDITORVIEW_H
#define STATESEDITORVIEW_H

#include <abstractview.h>

#include <qmlstate.h>

namespace QmlDesigner {


class StatesEditorModel;
class StatesEditorWidget;

class StatesEditorView : public AbstractView {
    Q_OBJECT

public:
    explicit StatesEditorView(QObject *parent = 0);
    ~StatesEditorView();

    void renameState(int nodeId,const QString &newName);
    bool validStateName(const QString &name) const;
    QString currentStateName() const;
    void setCurrentState(const QmlModelState &state);
    QmlModelState baseState() const;
    QmlModelStateGroup rootStateGroup() const;

    // AbstractView
    void modelAttached(Model *model) QTC_OVERRIDE;
    void modelAboutToBeDetached(Model *model) QTC_OVERRIDE;
    void propertiesRemoved(const QList<AbstractProperty>& propertyList) QTC_OVERRIDE;
    void variantPropertiesChanged(const QList<VariantProperty>& propertyList,
                                  PropertyChangeFlags propertyChange) QTC_OVERRIDE;

    void nodeAboutToBeRemoved(const ModelNode &removedNode) QTC_OVERRIDE;
    void nodeRemoved(const ModelNode &removedNode,
                     const NodeAbstractProperty &parentProperty,
                     PropertyChangeFlags propertyChange) QTC_OVERRIDE;
    void nodeAboutToBeReparented(const ModelNode &node,
                                 const NodeAbstractProperty &newPropertyParent,
                                 const NodeAbstractProperty &oldPropertyParent,
                                 AbstractView::PropertyChangeFlags propertyChange) QTC_OVERRIDE;
    void nodeReparented(const ModelNode &node,
                        const NodeAbstractProperty &newPropertyParent,
                        const NodeAbstractProperty &oldPropertyParent,
                        AbstractView::PropertyChangeFlags propertyChange) QTC_OVERRIDE;
    void nodeOrderChanged(const NodeListProperty &listProperty, const ModelNode &movedNode, int oldIndex) QTC_OVERRIDE;


    // AbstractView
    void currentStateChanged(const ModelNode &node) QTC_OVERRIDE;

    void scriptFunctionsChanged(const ModelNode &node, const QStringList &scriptFunctionList) QTC_OVERRIDE;
    void nodeIdChanged(const ModelNode &node, const QString &newId, const QString &oldId) QTC_OVERRIDE;
    void bindingPropertiesChanged(const QList<BindingProperty> &propertyList, PropertyChangeFlags propertyChange) QTC_OVERRIDE;
    void signalHandlerPropertiesChanged(const QVector<SignalHandlerProperty>& propertyList,PropertyChangeFlags propertyChange) QTC_OVERRIDE;
    void selectedNodesChanged(const QList<ModelNode> &selectedNodeList, const QList<ModelNode> &lastSelectedNodeList) QTC_OVERRIDE;

    void instancesPreviewImageChanged(const QVector<ModelNode> &nodeList) QTC_OVERRIDE;

    WidgetInfo widgetInfo() QTC_OVERRIDE;

    void nodeCreated(const ModelNode &createdNode) QTC_OVERRIDE;
    void propertiesAboutToBeRemoved(const QList<AbstractProperty> &propertyList) QTC_OVERRIDE;
    void rootNodeTypeChanged(const QString &type, int majorVersion, int minorVersion) QTC_OVERRIDE;
    void instancePropertyChange(const QList<QPair<ModelNode, PropertyName> > &propertyList) QTC_OVERRIDE;
    void instancesCompleted(const QVector<ModelNode> &completedNodeList) QTC_OVERRIDE;
    void instanceInformationsChange(const QMultiHash<ModelNode, InformationName> &informationChangeHash) QTC_OVERRIDE;
    void instancesRenderImageChanged(const QVector<ModelNode> &nodeList) QTC_OVERRIDE;
    void instancesChildrenChanged(const QVector<ModelNode> &nodeList) QTC_OVERRIDE;
    void instancesToken(const QString &tokenName, int tokenNumber, const QVector<ModelNode> &nodeVector) QTC_OVERRIDE;
    void nodeSourceChanged(const ModelNode &modelNode, const QString &newNodeSource) QTC_OVERRIDE;
    void rewriterBeginTransaction() QTC_OVERRIDE;
    void rewriterEndTransaction() QTC_OVERRIDE;
    void importsChanged(const QList<Import> &addedImports, const QList<Import> &removedImports) QTC_OVERRIDE;

public slots:
    void synchonizeCurrentStateFromWidget();
    void createNewState();
    void removeState(int nodeId);

private:
    StatesEditorWidget *statesEditorWidget() const;
    void resetModel();
    void addState();
    void duplicateCurrentState();
    void checkForWindow();

private:
    QWeakPointer<StatesEditorModel> m_statesEditorModel;
    QWeakPointer<StatesEditorWidget> m_statesEditorWidget;
    int m_lastIndex;
};

} // namespace QmlDesigner

#endif // STATESEDITORVIEW_H
