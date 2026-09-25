// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: area
    spacing: 1
    signal closeRequested(int index)
    property int editorRevision: 0
    function activeEditor() { editorRevision;return editors.itemAt(studio.active) }
    function focusSearch() { search.visible=true;Qt.callLater(function(){search.searchField.forceActiveFocus()}) }
    ListView {
        Layout.fillWidth: true; Layout.preferredHeight: Theme.controlHeight; orientation: ListView.Horizontal; spacing: Theme.xs; clip: true; model: studio.documents
        currentIndex: studio.active
        onCurrentIndexChanged: positionViewAtIndex(currentIndex,ListView.Contain)
        delegate: EditorTab { required property var modelData; required property int index; document: modelData; selected: index===studio.active; onSelectedByUser: studio.active=index; onCloseRequested: area.closeRequested(index) }
        ScrollBar.horizontal: ScrollBar {}
    }
    SearchBar { id: search; objectName: "findReplaceBar"; visible: false; Layout.fillWidth: true; editor: area.activeEditor(); onCloseRequested: {visible=false;let pane=area.activeEditor();if(pane)pane.editor.forceActiveFocus()} }
    StackLayout {
        Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: studio.active
        Repeater { id: editors; model: studio.documents; onItemAdded: area.editorRevision++; onItemRemoved: area.editorRevision++; EditorPane { required property var modelData; document: modelData } }
    }
    Label { visible: studio.documents.length===0; Layout.fillWidth: true; Layout.margins: Theme.lg; text: "Open a project file to start editing. Build uses your visible buffer; test scripts use saved project files."; wrapMode: Text.WordWrap; color: Theme.muted }
}
