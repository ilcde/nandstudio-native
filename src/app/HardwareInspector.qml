// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
                            id: inspector
                            function inputs() { let values={};for(let i=0;i<pinRows.count;i++){let row=pinRows.itemAt(i);if(row.modelData.direction==="input")values[row.modelData.name]=row.editText}return values }
                            property int revision: 0; Layout.fillWidth: true
                            Label { text: studio.state.chip + " Â· time " + studio.state.hardwareTime; font.bold: true }
                            RowLayout {
                                ActionButton { Layout.fillWidth: true; Layout.minimumWidth: 0; text: "Eval"; objectName: "hardwareEval"; enabled: !studio.busy; onClicked: studio.hardwareActionWithInputs("eval",inspector.inputs()) }
                                ActionButton { Layout.fillWidth: true; Layout.minimumWidth: 0; text: "Tick"; objectName: "hardwareTick"; enabled: !studio.busy && !studio.state.clockUp; onClicked: studio.hardwareActionWithInputs("tick",inspector.inputs()) }
                                ActionButton { Layout.fillWidth: true; Layout.minimumWidth: 0; text: "Tock"; objectName: "hardwareTock"; enabled: !studio.busy && studio.state.clockUp; onClicked: studio.hardwareActionWithInputs("tock",inspector.inputs()) }
                            }
                            Label { text: "PINS / INTERNAL WIRES"; opacity: 0.65; font.bold: true }
                            Repeater {
                                id: pinRows; model: studio.state.pins
                                RowLayout {
                                    required property var modelData
                                    property alias editText: pinField.text
                                    Layout.fillWidth: true
                                    Label { text: modelData.name + (modelData.width > 1 ? "[" + modelData.width + "]" : ""); Layout.fillWidth: true }
                                    Label { text: modelData.direction; opacity: 0.6; font.pixelSize: 11 }
                                    AppField { id: pinField; objectName: "pin_" + modelData.name; text: modelData.value; readOnly: modelData.direction !== "input" || studio.busy; Layout.preferredWidth: 80; onAccepted: studio.commitHardwareInputs(inspector.inputs()) }
                                }
                            }
                            Label { text: "COMPONENTS"; opacity: 0.65; font.bold: true }
                            ComboBox { id: componentChoice; implicitHeight: Theme.controlHeight; Layout.minimumWidth: 0; Layout.fillWidth: true; model: studio.state.components; textRole: "path" }
                            Label { Layout.fillWidth: true; elide: Text.ElideMiddle; text: componentChoice.currentIndex >= 0 ? studio.state.components[componentChoice.currentIndex].implementation : ""; opacity: 0.6 }
                            Repeater {
                                model: componentChoice.currentIndex >= 0 ? studio.state.components[componentChoice.currentIndex].pins : []
                                RowLayout {
                                    required property var modelData
                                    Label { text: modelData.name + " (" + modelData.direction + ")"; Layout.fillWidth: true; opacity: 0.7 }
                                    Label { text: modelData.value; font.family: "monospace" }
                                }
                            }
                            RowLayout {
                                visible: componentChoice.currentIndex >= 0 && (studio.state.components[componentChoice.currentIndex].words > 0 || ["PC","ARegister","DRegister"].indexOf(studio.state.components[componentChoice.currentIndex].implementation) >= 0)
                                SpinBox { id: componentIndex; from: 0; to: componentChoice.currentIndex >= 0 ? Math.max(0,studio.state.components[componentChoice.currentIndex].words-1) : 0; editable: true; Layout.fillWidth: true }
                                AppField {
                                    Layout.preferredWidth: 90
                                    property string variable: componentChoice.currentIndex >= 0 ? studio.state.components[componentChoice.currentIndex].path + "[" + componentIndex.value + "]" : ""
                                    text: { revision; return variable ? studio.hardwareValue(variable) : "" }
                                    enabled: !studio.busy
                                    onAccepted: studio.setHardware(variable,parseInt(text))
                                }
                            }
                            ActionButton { Layout.fillWidth: true; Layout.minimumWidth: 0; text: "Load current ASM/HACK into ROM"; enabled: !studio.busy && componentChoice.currentIndex >= 0 && studio.state.components[componentChoice.currentIndex].implementation === "ROM32K"; onClicked: studio.loadHardwareRom(studio.state.components[componentChoice.currentIndex].path) }
                            ActionButton { text: "Clock cycle"; objectName: "hardwareCycle"; Layout.fillWidth: true; enabled: !studio.busy; onClicked: studio.hardwareActionWithInputs("cycle",inspector.inputs()) }
                            HardwareTrace { Layout.fillWidth: true }
                            Label { text: "Load HDL explicitly replaces the running chip. Edits do not reset its state."; wrapMode: Text.WordWrap; Layout.fillWidth: true; font.pixelSize: 11; opacity: 0.65 }
                        }
