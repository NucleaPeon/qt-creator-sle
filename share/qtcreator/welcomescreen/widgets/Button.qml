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
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0

Button {
    id: button
    style: touchStyle
    iconSource: ""
    Component {
        id: touchStyle
        ButtonStyle {
            background: Item {

                Image {
                    id: icon
                    z: 1
                    x: 4
                    y: -6
                    width: 32
                    height: 32
                    source: button.iconSource
                    visible: button.iconSource != ""
                }

                implicitHeight: 30
                implicitWidth: 160

                Rectangle {
                    anchors.fill: parent
                    antialiasing: true
                    radius: 3

                    visible: !(button.pressed || button.checked)

                    gradient: Gradient {
                        GradientStop {
                            position: 0
                            color: "#f9f9f9"
                        }

                        GradientStop {
                            position: 0.49
                            color: "#f9f9f9"
                        }

                        GradientStop {
                            position: 0.5
                            color: "#eeeeee"
                        }

                        GradientStop {
                            position: 1
                            color: "#eeeeee"
                        }
                    }
                    border.color: "#737373"

                }

                Rectangle {
                    anchors.fill: parent
                    antialiasing: true
                    radius: 3

                    visible: button.pressed || button.checked

                    gradient: Gradient {
                        GradientStop {
                            position: 0.00;
                            color: "#4c4c4c";
                        }
                        GradientStop {
                            position: 0.49;
                            color: "#4c4c4c";
                        }
                        GradientStop {
                            position: 0.50;
                            color: "#424242";
                        }
                        GradientStop {
                            position: 1.00;
                            color: "#424242";
                        }
                    }
                    border.color: "#333333"

                }
            }

            label: Text {
                    x: button.iconSource != "" ? 38 : 14
                    renderType: Text.NativeRendering
                    verticalAlignment: Text.AlignVCenter
                    text: button.text
                    color:  button.pressed || button.checked ? "lightGray" : "black"
                    font.pixelSize: 15
                    font.bold: false
                    smooth: true
                }
        }
    }
}
