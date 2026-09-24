// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    id: toolbar
    signal folderRequested(); signal fileRequested(); signal settingsRequested()
    padding: Theme.sm
    implicitHeight: row.implicitHeight + padding*2
    background: Rectangle { color: Theme.surface }
    RowLayout {
        id: row; anchors.fill: parent; spacing: Theme.sm
        Label { text: "NandStudio"; visible: toolbar.width > 600; font.bold: true; font.pixelSize: Theme.bodySize+5; color: Theme.accent; Layout.rightMargin: Theme.sm }
        ActionButton { Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "Files"; onClicked: fileMenu.popup(); help: "Open files, save documents and manage the workspace" }
        ActionButton { Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "Save"; enabled: studio.active >= 0; onClicked: studio.documents[studio.active].save() }
        ActionButton { Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "Build"; enabled: !studio.busy; primary: true; onClicked: studio.build(); help: "Build the visible ASM or Jack buffer" }
        Item { Layout.fillWidth: true }
        BusyIndicator { visible: studio.busy && toolbar.width>500; running: studio.busy; Layout.preferredWidth: 32; Layout.preferredHeight: 32 }
        ActionButton { Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "More"; onClicked: moreMenu.popup() }
    }
    Menu {
        id: fileMenu
        MenuItem { text: "Open workspaceÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦"; onTriggered: toolbar.folderRequested() }
        MenuItem { text: "Open fileÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦"; onTriggered: toolbar.fileRequested() }
        Menu {
            id: recentMenu; title: "Recent workspaces"
            Instantiator { model: preferences.values.recentWorkspaces; delegate: MenuItem { required property string modelData; text: modelData; onTriggered: studio.openLocalWorkspace(modelData) } onObjectAdded: (index,object) => recentMenu.insertItem(index,object); onObjectRemoved: (index,object) => recentMenu.removeItem(object) }
        }
        MenuSeparator {}
        MenuItem { text: "Save all"; onTriggered: studio.saveAll() }
        MenuItem { text: "Refresh workspace"; onTriggered: studio.refreshWorkspace() }
    }
    Menu {
        id: moreMenu
        MenuItem { text: Theme.dark ? "Use light theme" : "Use dark theme"; onTriggered: preferences.set("dark",!Theme.dark) }
        MenuItem { text: "SettingsÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦"; onTriggered: toolbar.settingsRequested() }
    }
}
