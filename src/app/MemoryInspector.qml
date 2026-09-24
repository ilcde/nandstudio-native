// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    property int revision: 0
    Label { text: "MEMORY"; color: Theme.muted; font.bold: true }
    RowLayout {
        SpinBox { id: address; from: 0; to: 32767; editable: true; implicitHeight: Theme.controlHeight; Layout.fillWidth: true; Layout.minimumWidth: 0; Accessible.name: "Memory address" }
        AppField { text: { revision; return studio.memory(address.value) } Layout.preferredWidth: 100; validator: IntValidator { bottom: -32768; top: 32767 } Accessible.name: "Memory value"; onAccepted: if(acceptableInput)studio.setMemory(address.value,parseInt(text)) }
    }
    Label { text: "Press Return to write the selected RAM word."; color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
}
