// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: area
    spacing: 1
    signal closeRequested(int index)
    function activeEditor() { return editors.itemAt(studio.active) }
    function focusSearch() { search.searchField.forceActiveFocus() }
    ListView {
        Layout.fillWidth: true; Layout.preferredHeight: Theme.controlHeight; orientation: ListView.Horizontal; spacing: Theme.xs; clip: true; model: studio.documents
        currentIndex: studio.active
        onCurrentIndexChanged: positionViewAtIndex(currentIndex,ListView.Contain)
        delegate: EditorTab { required property var modelData; required property int index; document: modelData; selected: index===studio.active; onSelectedByUser: studio.active=index; onCloseRequested: area.closeRequested(index) }
        ScrollBar.horizontal: ScrollBar {}
    }
    SearchBar { id: search; Layout.fillWidth: true; editor: area.activeEditor() }
    StackLayout {
        Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: studio.active
        Repeater { id: editors; model: studio.documents; EditorPane { required property var modelData; document: modelData } }
    }
    Label { visible: studio.documents.length===0; Layout.fillWidth: true; Layout.margins: Theme.lg; text: "Open a project file to start editing. Build uses your visible buffer; test scripts use saved project files."; wrapMode: Text.WordWrap; color: Theme.muted }
}
