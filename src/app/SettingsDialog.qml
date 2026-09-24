// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
AppDialog {
    id: dialog; title: "Settings"; objectName: "settingsDialog"
    contentItem: ScrollView {
        id: scroll; implicitWidth: 360; implicitHeight: Math.min(420,body.implicitHeight); contentWidth: availableWidth; clip: true
        ColumnLayout {
            id: body; width: scroll.availableWidth; spacing: Theme.sm
            Switch { text: "Dark theme"; checked: preferences.values.dark; onToggled: preferences.set("dark",checked) }
            Label { text: "Editor font size" }
            SpinBox { from: 10; to: 32; value: preferences.values.fontSize; editable: true; onValueModified: preferences.set("fontSize",value); Layout.fillWidth: true }
            Label { text: "Font family" }
            AppField { text: preferences.values.fontFamily; Layout.fillWidth: true; onAccepted: preferences.set("fontFamily",text) }
            Label { text: "Indent width" }
            SpinBox { from: 1; to: 8; value: preferences.values.indentWidth; onValueModified: preferences.set("indentWidth",value); Layout.fillWidth: true }
            Switch { text: "Indent with tabs"; checked: preferences.values.indentTabs; onToggled: preferences.set("indentTabs",checked) }
            Switch { text: "Wrap editor lines"; checked: preferences.values.wordWrap; onToggled: preferences.set("wordWrap",checked) }
            Label { text: "Interface scale" }
            Slider { from: 1; to: 1.5; stepSize: 0.1; value: preferences.values.uiScale; onMoved: preferences.set("uiScale",value); Layout.fillWidth: true }
            Label { text: "Autosave interval (seconds; 0 disables)"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            SpinBox { from: 0; to: 600; stepSize: 5; value: preferences.values.autosaveSeconds; onValueModified: preferences.set("autosaveSeconds",value); Layout.fillWidth: true }
            Label { text: "Recovery remains enabled. Autosave stops when external changes conflict with your buffer."; color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
    }
    footer: ActionButton { text: "Done"; onClicked: dialog.accept() }
}
