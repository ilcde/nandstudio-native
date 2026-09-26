// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    id: pane
    property int revision: 0
    padding: Theme.sm
    background: Rectangle { color: Theme.surface }
    ScrollView {
        id: scroll; anchors.fill: parent; contentWidth: availableWidth; clip: true
        ColumnLayout {
            width: scroll.availableWidth; spacing: Theme.md
            Label { text: studio.state.mode; font.pixelSize: Theme.bodySize+5; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WordWrap }
            Label { text: studio.hardwareMessage; visible: text.length>0; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.accent }
            ActionButton { objectName: "initialHdlEval"; text: "Load & Eval HDL"; visible: !studio.state.hardware; enabled: !studio.busy; onClicked: studio.evaluateHardwareWithInputs({}); Layout.fillWidth: true }
            GridLayout {
                columns: 3; Layout.fillWidth: true
                ActionButton { text: "Load CPU"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.loadCpu() }
                ActionButton { text: "Load VM"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.loadVm() }
                ActionButton { text: "Load HDL"; objectName: "loadHdl"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.loadHardware() }
                ActionButton { text: "Step"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.step(1) }
                ActionButton { text: "Run 10k"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.step(10000) }
                ActionButton { text: "Stop"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: studio.busy; onClicked: studio.cancel() }
            }
            ActionButton { text: "Restart"; visible: !studio.state.hardware; enabled: !studio.busy; onClicked: studio.reset() }
            GridLayout {
                visible: !studio.state.hardware; columns: 2; Layout.fillWidth: true
                Repeater { model: ["A","D","PC","SP","time"]; RowLayout { required property string modelData; Layout.fillWidth: true; Label { text: modelData; color: Theme.muted } Label { text: studio.state[modelData]; font.family: "monospace"; font.bold: true; Layout.fillWidth: true; elide: Text.ElideLeft } } }
            }
            HardwareInspector { visible: studio.state.hardware; Layout.fillWidth: true; revision: pane.revision }
            VmInspector { visible: studio.state.mode==="VM Emulator"; Layout.fillWidth: true; revision: pane.revision }
            ScreenView { Layout.fillWidth: true; revision: pane.revision }
            MemoryInspector { visible: !studio.state.hardware; Layout.fillWidth: true; revision: pane.revision }
            GridLayout {
                columns: 3; Layout.fillWidth: true
                ActionButton { text: "CPU test"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.test(false) }
                ActionButton { text: "VM test"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.test(true) }
                ActionButton { text: "HDL test"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: !studio.busy; onClicked: studio.testHardware() }
            }
        }
    }
}
