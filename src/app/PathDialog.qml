// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
AppDialog {
    id: dialog
    objectName: "pathDialog"
    property alias value: field.text
    property string prompt: "Project-relative filename"
    property string actionText: "Create"
    property string failure: ""
    signal submitted(string value)
    onOpened: { failure=""; field.forceActiveFocus(); field.selectAll() }
    contentItem: ColumnLayout {
        spacing: Theme.sm
        Label { text: dialog.prompt; color: Theme.muted; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        AppField { id: field; objectName: "pathField"; placeholderText: "Name.jack"; Layout.fillWidth: true; Layout.minimumWidth: 0; onAccepted: if(text.trim().length)dialog.submitted(text) }
        Label { text: dialog.failure; visible: text.length > 0; color: Theme.danger; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    }
    footer: RowLayout {
        spacing: Theme.sm
        Item { Layout.fillWidth: true }
        ActionButton { text: "Cancel"; objectName: "pathCancel"; onClicked: dialog.reject(); Layout.bottomMargin: Theme.md }
        ActionButton { text: dialog.actionText; objectName: "pathSubmit"; primary: true; enabled: field.text.trim().length > 0; onClicked: dialog.submitted(field.text); Layout.rightMargin: Theme.md; Layout.bottomMargin: Theme.md }
    }
}
