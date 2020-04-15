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
import HelperWidgets 2.0
import QtQuick.Layouts 1.0

Column {
    anchors.left: parent.left
    anchors.right: parent.right


    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("Color")


        ColorEditor {
            caption: qsTr("Color")
            backendendValue: backendValues.textColor
            supportGradient: false
        }
    }

    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("Text Area")

        SectionLayout {

            Label {
                text: qsTr("Text")
                toolTip:  qsTr("The text shown on the text area")
            }

            SecondColumnLayout {
                LineEdit {
                    backendValue: backendValues.text
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Read only")
                toolTip: qsTr("Determines whether the text area is read only.")
            }

            SecondColumnLayout {
                CheckBox {
                    backendValue: backendValues.readOnly
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Document margins")
                toolTip: qsTr("The margins of the text area")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.documentMargins
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }


            Label {
                text: qsTr("Frame width")
                toolTip: qsTr("The width of the frame")
            }

            SectionLayout {
                SpinBox {
                    backendValue: backendValues.frameWidth
                    minimumValue: 0;
                    maximumValue: 1000;
                    decimals: 0
                }

                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Contents frame")
                toolTip: qsTr("Determines whether the frame around contents is shown.")
            }

            SecondColumnLayout {
                CheckBox {
                    backendValue: backendValues.frameAroundContents
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

        }

    }

    FontSection {
        showStyle: false
    }


    Section {
        anchors.left: parent.left
        anchors.right: parent.right
        caption: qsTr("Focus Handling")

        SectionLayout {

            Label {
                text: qsTr("Highlight on focus")
                toolTip: qsTr("Determines whether the text area is highlighted on focus.")
            }

            SecondColumnLayout {
                CheckBox {
                    backendValue: backendValues.highlightOnFocus
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Tab changes focus")
                toolTip: qsTr("Determines whether tab changes the focus of the text area.")
            }

            SecondColumnLayout {
                CheckBox {
                    backendValue: backendValues.tabChangesFocus
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Focus on press")
                toolTip: qsTr("Determines whether the text area gets focus if pressed.")
            }

            SecondColumnLayout {
                CheckBox {
                    backendValue: backendValues.activeFocusOnPress
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

        }
    }
}
