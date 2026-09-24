// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    id: pane
    signal createRequested(); signal folderRequested(); signal opened(); signal renameRequested(string path); signal trashRequested(string path)
    padding: Theme.sm
    background: Rectangle { color: Theme.surface }
    ColumnLayout {
        anchors.fill: parent; spacing: Theme.sm
        Label { text: "WORKSPACE"; color: Theme.muted; font.bold: true; font.letterSpacing: 1 }
        Label { text: studio.workspace || "Open a project folder"; elide: Text.ElideMiddle; Layout.fillWidth: true; Layout.minimumWidth: 0; ToolTip.text: text; ToolTip.visible: hover.hovered; HoverHandler { id: hover } }
        RowLayout { Layout.fillWidth: true; ActionButton { text: "New file"; objectName: "newFileButton"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: studio.workspace.length>0; onClicked: pane.createRequested() } ActionButton { text: "Folder"; Layout.fillWidth: true; Layout.minimumWidth: 0; enabled: studio.workspace.length>0; onClicked: pane.folderRequested() } }
        AppField { Layout.fillWidth: true; Layout.minimumWidth: 0; placeholderText: "Search project"; onAccepted: studio.findInProject(text) }
        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true; model: studio.files
            ScrollBar.vertical: ScrollBar {}
            delegate: ItemDelegate {
                required property var modelData
                width: ListView.view.width; height: Math.max(Theme.controlHeight, label.implicitHeight+Theme.sm*2)
                contentItem: RowLayout {
                    Label { id: label; text: modelData.name; elide: Text.ElideMiddle; verticalAlignment: Text.AlignVCenter; color: Theme.text; Layout.fillWidth: true; Layout.minimumWidth: 0 }
                    ToolButton { text: "⋯"; implicitWidth: Theme.controlHeight; implicitHeight: Theme.controlHeight; Accessible.name: "Actions for "+modelData.name; onClicked: entryMenu.popup() }
                }
                Menu { id: entryMenu; MenuItem { text: "Rename…"; onTriggered: pane.renameRequested(modelData.name) } MenuItem { text: "Move to recovery trash…"; onTriggered: pane.trashRequested(modelData.name) } }
                background: Rectangle { radius: Theme.radius; color: parent.hovered || parent.activeFocus ? Theme.raised : "transparent"; border.color: parent.activeFocus ? Theme.accent : "transparent" }
                onClicked: { studio.open(modelData.path); pane.opened() }
                ToolTip.visible: hovered; ToolTip.text: modelData.name
                Accessible.name: modelData.name
            }
        }
    }
}
