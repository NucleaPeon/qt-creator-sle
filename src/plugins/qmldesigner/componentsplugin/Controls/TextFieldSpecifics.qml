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
        caption: qsTr("Text Field")

        SectionLayout {

            Label {
                text: qsTr("Text")
                toolTip:  qsTr("The text shown on the text field")
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
                text:  qsTr("Placeholder text")
                toolTip: qsTr("The placeholder text")
            }

            SecondColumnLayout {
                LineEdit {
                    backendValue: backendValues.placeholderText
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Read only")
                toolTip: qsTr("Determines whether the text field is read only.")
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
                text: qsTr("Password mode")
                toolTip: qsTr("Determines whether the text field is in password mode.")
            }

            SecondColumnLayout {
                CheckBox {
                    backendValue: backendValues.passwordMode
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Input mask")
                toolTip: qsTr("Restricts the valid text in the text field.")
            }

            SecondColumnLayout {
                LineEdit {
                    backendValue: backendValues.inputMask
                    implicitWidth: 180
                }
                ExpandingSpacer {

                }
            }

            Label {
                text: qsTr("Echo mode")
                toolTip: qsTr("Specifies how the text is displayed in the text field.")
            }

            SecondColumnLayout {
                ComboBox {
                    useInteger: true
                    backendValue: backendValues.echoMode
                    implicitWidth: 180
                    model:  ["Normal", "Password", "PasswordEchoOnEdit", "NoEcho"]
                    scope: "TextInput"
                }
                ExpandingSpacer {

                }
            }


        }

    }

    FontSection {
        showStyle: false
    }
}
