// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    id: toolbar
    objectName: "appToolbar"
    signal folderRequested(); signal fileRequested(); signal settingsRequested(); signal searchRequested(); signal exportRequested()
    property real safeTop: SafeArea.margins.top
    // Button menus belong below the header, never centered on its short height.
    function openMenu(menu, button) {
        const position=button.mapToItem(Overlay.overlay,0,button.height)
        menu.x=Math.max(menu.leftMargin,Math.min(position.x,Overlay.overlay.width-menu.width-menu.rightMargin))
        menu.y=Math.max(menu.topMargin,position.y+Theme.xs)
        menu.open()
    }
    padding: Theme.sm
    topPadding: Theme.sm + safeTop
    leftPadding: Math.max(Theme.sm,SafeArea.margins.left)
    rightPadding: Math.max(Theme.sm,SafeArea.margins.right)
    implicitHeight: row.implicitHeight + topPadding + bottomPadding
    background: Rectangle { color: Theme.surface }
    RowLayout {
        id: row; anchors.fill: parent; spacing: Theme.sm
        Label { text: "NandStudio"; visible: toolbar.width > 600; font.bold: true; font.pixelSize: Theme.bodySize+5; color: Theme.accent; Layout.rightMargin: Theme.sm }
        ActionButton { id: filesButton; objectName: "filesButton"; Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "Files"; onClicked: toolbar.openMenu(fileMenu,filesButton); help: "Open files, save documents and manage the workspace" }
        ActionButton { objectName: "saveButton"; Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "Save"; enabled: studio.active >= 0; onClicked: studio.documents[studio.active].save() }
        ActionButton { objectName: "buildButton"; Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "Build"; enabled: !studio.busy; primary: true; onClicked: studio.build(); help: "Build the visible ASM or Jack buffer, or load HDL" }
        Item { Layout.fillWidth: true }
        BusyIndicator { visible: studio.busy && toolbar.width>500; running: studio.busy; Layout.preferredWidth: 32; Layout.preferredHeight: 32 }
        ActionButton { id: moreButton; objectName: "moreButton"; Layout.minimumWidth: 0; Layout.fillWidth: toolbar.width < 500; text: "More"; onClicked: toolbar.openMenu(moreMenu,moreButton) }
    }
    Menu {
        id: fileMenu; objectName: "filesMenu"
        parent: Overlay.overlay; popupType: Popup.Item
        width: Math.min(320*Theme.scale,parent.width-leftMargin-rightMargin)
        topMargin: toolbar.height; bottomMargin: Math.max(Theme.sm,Overlay.overlay.SafeArea.margins.bottom)
        leftMargin: Math.max(Theme.sm,toolbar.SafeArea.margins.left); rightMargin: Math.max(Theme.sm,toolbar.SafeArea.margins.right)
        MenuItem { objectName: "openWorkspaceMenuItem"; text: "Open workspace\u2026"; onTriggered: toolbar.folderRequested() }
        MenuItem { objectName: "openFileMenuItem"; text: "Open file\u2026"; onTriggered: toolbar.fileRequested() }
        Menu {
            id: recentMenu; title: "Recent workspaces"
            Instantiator { model: preferences.values.recentWorkspaces; delegate: MenuItem { required property string modelData; text: modelData; onTriggered: studio.openLocalWorkspace(modelData) } onObjectAdded: (index,object) => recentMenu.insertItem(index,object); onObjectRemoved: (index,object) => recentMenu.removeItem(object) }
        }
        MenuSeparator {}
        MenuItem { text: "Save all"; onTriggered: studio.saveAll() }
        MenuItem { text: "Refresh workspace"; onTriggered: studio.refreshWorkspace() }
        MenuItem { objectName: "exportWorkspaceMenuItem"; text: "Export workspace copy\u2026"; enabled: studio.workspace.length>0 && !studio.busy; onTriggered: toolbar.exportRequested() }
    }
    Menu {
        id: moreMenu; objectName: "moreMenu"
        parent: Overlay.overlay; popupType: Popup.Item
        width: Math.min(320*Theme.scale,parent.width-leftMargin-rightMargin)
        topMargin: toolbar.height; bottomMargin: Math.max(Theme.sm,Overlay.overlay.SafeArea.margins.bottom)
        leftMargin: Math.max(Theme.sm,toolbar.SafeArea.margins.left); rightMargin: Math.max(Theme.sm,toolbar.SafeArea.margins.right)
        MenuItem { objectName: "findReplaceMenuItem"; text: "Find and replace\u2026"; enabled: studio.active >= 0; onTriggered: toolbar.searchRequested() }
        MenuItem { text: Theme.dark ? "Use light theme" : "Use dark theme"; onTriggered: preferences.set("dark",!Theme.dark) }
        MenuItem { objectName: "settingsMenuItem"; text: "Settings\u2026"; onTriggered: toolbar.settingsRequested() }
        MenuSeparator {}
        MenuItem { enabled: false; text: "NandStudio " + Qt.application.version }
    }
}
