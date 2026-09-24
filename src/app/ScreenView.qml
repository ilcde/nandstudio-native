// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
 property int revision: 0
                        Label { text: "SCREEN / KEYBOARD"; opacity: 0.6; font.bold: true }
                        Image {
                            id: screen
                            Layout.fillWidth: true; Layout.preferredHeight: width/2
                            source: "image://screen/"+revision; cache: false; smooth: false; fillMode: Image.PreserveAspectFit
                            activeFocusOnTab: true
                            MouseArea { anchors.fill: parent; onClicked: screen.forceActiveFocus() }
                            Rectangle { anchors.fill: parent; color: "transparent"; border.color: screen.activeFocus ? "#6baaf7" : "#697484"; border.width: screen.activeFocus ? 2 : 1 }
                            Keys.onPressed: event => { let keys={};keys[Qt.Key_Return]=128;keys[Qt.Key_Backspace]=129;keys[Qt.Key_Left]=130;keys[Qt.Key_Up]=131;keys[Qt.Key_Right]=132;keys[Qt.Key_Down]=133;keys[Qt.Key_Home]=134;keys[Qt.Key_End]=135;keys[Qt.Key_PageUp]=136;keys[Qt.Key_PageDown]=137;keys[Qt.Key_Insert]=138;keys[Qt.Key_Delete]=139;keys[Qt.Key_Escape]=140;let c=keys[event.key]||(event.text.length ? event.text.toUpperCase().charCodeAt(0) : 0);studio.key(c);event.accepted=true }
                            Keys.onReleased: event => { studio.key(0);event.accepted=true }
                            onActiveFocusChanged: if(!activeFocus)studio.key(0)
                        }
                        Label { text: "Focus screen to send keys"; opacity: 0.6; font.pixelSize: 12 }
                        RowLayout {
                            Repeater {
                                model: [{label:"←",key:130},{label:"↑",key:131},{label:"↓",key:133},{label:"→",key:132},{label:"↵",key:128}]
                                ActionButton { required property var modelData; text: modelData.label; Layout.fillWidth: true; Layout.minimumWidth: 0; onPressed: studio.key(modelData.key); onReleased: studio.key(0); onCanceled: studio.key(0) }
                            }
                        }

}
