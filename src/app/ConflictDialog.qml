// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
AppDialog {
    id: dialog; title: "File changed on disk"; objectName: "conflictDialog"
    property var document: null
    contentItem: ColumnLayout {
        Label { text: dialog.document ? dialog.document.path : ""; elide: Text.ElideMiddle; Layout.fillWidth: true; Layout.minimumWidth: 0 }
        Label { text: "Your buffer is preserved. Reload discards your current edits. Overwrite replaces only the disk version shown below; a newer external change will stop the save."; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        ScrollView { Layout.fillWidth: true; Layout.preferredHeight: Math.min(160,dialog.parent.height/4); clip: true; TextArea { text: dialog.document ? dialog.document.diskText : ""; readOnly: true; selectByMouse: true; wrapMode: TextEdit.Wrap; color: Theme.text } }
    }
    footer: GridLayout {
        columns: 3; columnSpacing: Theme.sm
        ActionButton { text: "Keep editing"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: dialog.reject() }
        ActionButton { text: "Reload disk"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: {if(dialog.document.resolveConflict("reload"))dialog.accept()} }
        ActionButton { text: "Overwrite"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: {if(dialog.document.resolveConflict("overwrite"))dialog.accept()} }
    }
}
