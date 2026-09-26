// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: inspector
    objectName: "vmInspector"
    property int revision: 0
    property var snapshot: { revision; return studio.vmInspection() }
    spacing: Theme.sm
    Label { text: "VM INSTRUCTION"; font.bold: true; color: Theme.muted }
    Label { objectName: "vmInstruction"; text: inspector.snapshot.instruction || ""; font.family: "monospace"; Layout.fillWidth: true; wrapMode: Text.Wrap }
    Label { text: inspector.snapshot.source || ""; color: Theme.muted; Layout.fillWidth: true; elide: Text.ElideMiddle }
    Flow {
        Layout.fillWidth: true; spacing: Theme.sm
        Repeater { model: inspector.snapshot.segments || []; Label { required property var modelData; text: modelData.name+"="+modelData.value; font.family: "monospace" } }
    }
    Label { text: "CALLS / CURRENT FUNCTION"; font.bold: true; color: Theme.muted }
    Repeater { model: inspector.snapshot.calls || []; Label { required property string modelData; text: modelData; Layout.fillWidth: true; elide: Text.ElideMiddle } }
    Label { text: inspector.snapshot.function || ""; Layout.fillWidth: true; wrapMode: Text.Wrap; color: Theme.accent }
    Label { text: "STACK (last 16 words above address 255)"; font.bold: true; color: Theme.muted; Layout.fillWidth: true; wrapMode: Text.Wrap }
    Label { visible: inspector.snapshot.validSP === false; text: "SP is outside RAM"; color: Theme.accent }
    Repeater {
        model: inspector.snapshot.stack || []
        RowLayout {
            required property var modelData
            Label { text: modelData.address; color: Theme.muted; Layout.fillWidth: true }
            Label { text: modelData.value; font.family: "monospace" }
        }
    }
}
