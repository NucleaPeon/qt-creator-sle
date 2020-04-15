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

#ifndef QUICK2PROERTYEDITORVIEW_H
#define QUICK2PROERTYEDITORVIEW_H

#include <QWidget>
#include <QUrl>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>

QT_BEGIN_NAMESPACE
class QQmlError;
class QQmlComponent;
QT_END_NAMESPACE

namespace QmlDesigner {

class Quick2PropertyEditorView : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource DESIGNABLE true)
public:
    explicit Quick2PropertyEditorView(QWidget *parent = 0);

    QUrl source() const;
    void setSource(const QUrl&);

    QQmlEngine* engine();
    QQmlContext* rootContext();

    enum Status { Null, Ready, Loading, Error };
    Status status() const;

    static void registerQmlTypes();

signals:
    void statusChanged(Quick2PropertyEditorView::Status);

protected:
    void execute();

private Q_SLOTS:
    void continueExecute();

private:
     QWidget *m_containerWidget;
     QUrl m_source;
     QQuickView m_view;
     QWeakPointer<QQmlComponent> m_component;

};

} //QmlDesigner

#endif // QUICK2PROERTYEDITORVIEW_H
