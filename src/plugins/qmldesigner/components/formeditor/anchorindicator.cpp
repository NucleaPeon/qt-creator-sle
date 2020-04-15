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

#include "anchorindicator.h"

#include "qmlanchors.h"
#include "anchorindicatorgraphicsitem.h"
#include <QGraphicsScene>
#include <QGraphicsPathItem>

namespace QmlDesigner {

AnchorIndicator::AnchorIndicator(LayerItem *layerItem)
    : m_layerItem(layerItem)
{
}

AnchorIndicator::AnchorIndicator()
{
}

AnchorIndicator::~AnchorIndicator()
{
    clear();
}

void AnchorIndicator::show()
{
    if (m_indicatorTopShape)
        m_indicatorTopShape->show();

    if (m_indicatorBottomShape)
        m_indicatorBottomShape->show();

    if (m_indicatorLeftShape)
        m_indicatorLeftShape->show();

    if (m_indicatorRightShape)
        m_indicatorRightShape->show();
}

void AnchorIndicator::hide()
{
    if (m_indicatorTopShape)
        m_indicatorTopShape->hide();

    if (m_indicatorBottomShape)
        m_indicatorBottomShape->hide();

    if (m_indicatorLeftShape)
        m_indicatorLeftShape->hide();

    if (m_indicatorRightShape)
        m_indicatorRightShape->hide();
}

void AnchorIndicator::clear()
{
  delete m_indicatorTopShape;
  delete m_indicatorBottomShape;
  delete m_indicatorLeftShape;
  delete m_indicatorRightShape;
}

void AnchorIndicator::setItems(const QList<FormEditorItem *> &itemList)
{
    clear();

    if (itemList.count() == 1) {
        m_formEditorItem = itemList.first();
        QmlItemNode sourceQmlItemNode = m_formEditorItem->qmlItemNode();
        if (!sourceQmlItemNode.modelNode().isRootNode()) {
            QmlAnchors qmlAnchors = sourceQmlItemNode.anchors();

            if (qmlAnchors.modelHasAnchor(AnchorLine::Top)) {
                m_indicatorTopShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                m_indicatorTopShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Top),
                                                           qmlAnchors.modelAnchor(AnchorLine::Top));
            }

            if (qmlAnchors.modelHasAnchor(AnchorLine::Bottom)) {
                m_indicatorBottomShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                m_indicatorBottomShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Bottom),
                                                              qmlAnchors.modelAnchor(AnchorLine::Bottom));
            }

            if (qmlAnchors.modelHasAnchor(AnchorLine::Left)) {
                m_indicatorLeftShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                m_indicatorLeftShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Left),
                                                            qmlAnchors.modelAnchor(AnchorLine::Left));
            }

            if (qmlAnchors.modelHasAnchor(AnchorLine::Right)) {
                m_indicatorRightShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                m_indicatorRightShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Right),
                                                             qmlAnchors.modelAnchor(AnchorLine::Right));
            }
        }
    }
}

void AnchorIndicator::updateItems(const QList<FormEditorItem *> &itemList)
{
    foreach (FormEditorItem *formEditorItem, itemList) {
        if (formEditorItem == m_formEditorItem) {
            QmlItemNode sourceQmlItemNode = m_formEditorItem->qmlItemNode();
            if (!sourceQmlItemNode.modelNode().isRootNode()) {
                QmlAnchors qmlAnchors = formEditorItem->qmlItemNode().anchors();

                if (qmlAnchors.modelHasAnchor(AnchorLine::Top)) {
                    if (m_indicatorTopShape.isNull())
                        m_indicatorTopShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                    m_indicatorTopShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Top),
                                                               qmlAnchors.modelAnchor(AnchorLine::Top));
                } else {
                    delete m_indicatorTopShape;
                }

                if (qmlAnchors.modelHasAnchor(AnchorLine::Bottom)) {
                    if (m_indicatorBottomShape.isNull())
                        m_indicatorBottomShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                    m_indicatorBottomShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Bottom),
                                                                  qmlAnchors.modelAnchor(AnchorLine::Bottom));
                } else {
                    delete m_indicatorBottomShape;
                }

                if (qmlAnchors.modelHasAnchor(AnchorLine::Left)) {
                    if (m_indicatorLeftShape.isNull())
                        m_indicatorLeftShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                    m_indicatorLeftShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Left),
                                                                qmlAnchors.modelAnchor(AnchorLine::Left));
                } else {
                    delete m_indicatorLeftShape;
                }

                if (qmlAnchors.modelHasAnchor(AnchorLine::Right)) {
                    if (m_indicatorRightShape.isNull())
                        m_indicatorRightShape = new AnchorIndicatorGraphicsItem(m_layerItem.data());
                    m_indicatorRightShape->updateAnchorIndicator(AnchorLine(sourceQmlItemNode, AnchorLine::Right),
                                                                 qmlAnchors.modelAnchor(AnchorLine::Right));
                } else {
                    delete m_indicatorRightShape;
                }
            }

            return;
        }
    }
}

} // namespace QmlDesigner
