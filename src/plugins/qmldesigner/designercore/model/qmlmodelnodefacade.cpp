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

#include "qmlmodelnodefacade.h"
#include "nodeinstanceview.h"

namespace QmlDesigner {

QmlModelNodeFacade::QmlModelNodeFacade() : m_modelNode(ModelNode())
{}

AbstractView *QmlModelNodeFacade::view() const
{
    if (modelNode().isValid())
        return modelNode().view();
    else
        return 0;
}

NodeInstanceView *QmlModelNodeFacade::nodeInstanceView(const ModelNode &modelNode)
{
    return modelNode.model()->nodeInstanceView();
}

NodeInstanceView *QmlModelNodeFacade::nodeInstanceView() const
{
    return nodeInstanceView(m_modelNode);
}


QmlModelNodeFacade::QmlModelNodeFacade(const ModelNode &modelNode) : m_modelNode(modelNode)
{}

QmlModelNodeFacade::~QmlModelNodeFacade()
{}

QmlModelNodeFacade::operator ModelNode() const
{
    return m_modelNode;
}

ModelNode QmlModelNodeFacade::modelNode()
{
    return m_modelNode;
}

const ModelNode QmlModelNodeFacade::modelNode() const
{
    return m_modelNode;
}

bool QmlModelNodeFacade::isValid() const
{
    return isValidQmlModelNodeFacade(m_modelNode);
}

bool QmlModelNodeFacade::isValidQmlModelNodeFacade(const ModelNode &modelNode)
{
    return modelNode.isValid()
            && nodeInstanceView(modelNode)
            && nodeInstanceView(modelNode)->hasInstanceForModelNode(modelNode)
            && nodeInstanceView(modelNode)->instanceForModelNode(modelNode).isValid();
}

bool QmlModelNodeFacade::isRootNode() const
{
    return modelNode().isRootNode();
}
} //QmlDesigner
