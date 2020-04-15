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

import QtQuick 2.1
import widgets 1.0

Rectangle {
    id: rectangle1
    width: parent.width
    height: Math.max(sessions.height, recentProjects.height)

    Item {
        id: canvas

        x: 12
        y: 0

        anchors.bottomMargin: 0
        anchors.fill: parent
        anchors.topMargin: 0

        RecentProjects {
            x: 428

            id: recentProjects

            anchors.leftMargin: 12
            anchors.left: recentProjectsTitle.left

            anchors.top: recentProjectsTitle.bottom
            anchors.topMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 60

            model: projectList
        }

        NativeText {
            id: sessionsTitle

            x: 32
            y: 128

            color: "#535353"
            text: qsTr("Sessions")
            font.pixelSize: 16
            font.family: "Helvetica"
            font.bold: true
        }

        NativeText {
            id: recentProjectsTitle
            x: 406

            y: 128
            color: "#535353"
            text: qsTr("Recent Projects")
            anchors.left: sessionsTitle.right
            anchors.leftMargin: 280
            font.bold: true
            font.family: "Helvetica"
            font.pixelSize: 16
        }

        Item {
            id: actions
            x: pageCaption.x + pageCaption.textOffset

            y: 295
            width: 140
            height: 70

            anchors.topMargin: 42
            anchors.top: sessions.bottom
        }

        Button {
            y: 51
            text: qsTr("New Project")
            anchors.left: sessionsTitle.left
            onClicked: projectWelcomePage.newProject();
            iconSource: "widgets/images/new.png"

        }

        Button {
            y: 51
            text: qsTr("Open Project")
            anchors.left: recentProjectsTitle.left
            onClicked: projectWelcomePage.openProject();
            iconSource: "widgets/images/open.png"
        }

        Sessions {
            id: sessions

            x: 96
            y: 144
            width: 274

            anchors.leftMargin: 12
            anchors.left: sessionsTitle.left
            anchors.right: recentProjectsTitle.left
            anchors.rightMargin: 40
            anchors.top: sessionsTitle.bottom
            anchors.topMargin: 20

            model: sessionList
        }
    }
}
