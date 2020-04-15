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

#include "resizeindicator.h"

#include "formeditoritem.h"

namespace QmlDesigner {

ResizeIndicator::ResizeIndicator(LayerItem *layerItem)
    : m_layerItem(layerItem)
{
    Q_ASSERT(layerItem);
}

ResizeIndicator::~ResizeIndicator()
{
    m_itemControllerHash.clear();
}

void ResizeIndicator::show()
{
    QHashIterator<FormEditorItem*, ResizeController> itemControllerIterator(m_itemControllerHash);
    while (itemControllerIterator.hasNext()) {
        ResizeController controller = itemControllerIterator.next().value();
        controller.show();
    }
}
void ResizeIndicator::hide()
{
    QHashIterator<FormEditorItem*, ResizeController> itemControllerIterator(m_itemControllerHash);
    while (itemControllerIterator.hasNext()) {
        ResizeController controller = itemControllerIterator.next().value();
        controller.hide();
    }
}

void ResizeIndicator::clear()
{
    m_itemControllerHash.clear();
}

bool itemIsResizable(const QmlItemNode &qmlItemNode)
{
    return qmlItemNode.isValid()
            && qmlItemNode.instanceIsResizable()
            && qmlItemNode.modelIsMovable()
            && qmlItemNode.modelIsResizable()
            && !qmlItemNode.instanceHasRotationTransform()
            && !qmlItemNode.instanceIsInLayoutable();
}

void ResizeIndicator::setItems(const QList<FormEditorItem*> &itemList)
{
    clear();

    foreach (FormEditorItem* item, itemList) {
        if (item && itemIsResizable(item->qmlItemNode())) {
            ResizeController controller(m_layerItem, item);
            m_itemControllerHash.insert(item, controller);
        }
    }
}

void ResizeIndicator::updateItems(const QList<FormEditorItem*> &itemList)
{
    foreach (FormEditorItem* item, itemList) {
        if (m_itemControllerHash.contains(item)) {
            if (!item || !itemIsResizable(item->qmlItemNode())) {
                m_itemControllerHash.take(item);
            } else {
                ResizeController controller(m_itemControllerHash.value(item));
                controller.updatePosition();
            }
        } else if (item && itemIsResizable(item->qmlItemNode())) {
            ResizeController controller(m_layerItem, item);
            m_itemControllerHash.insert(item, controller);
        }
    }
}

}