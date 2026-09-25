// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    id: bar
    property var editor
    property alias searchField: search
    signal closeRequested()
    property string feedback: ""
    padding: Theme.sm
    implicitHeight: layout.implicitHeight + padding*2
    background: Rectangle { color: Theme.surface }
    GridLayout {
        id: layout; anchors.fill: parent; columns: bar.width < 620 ? 2 : 4; columnSpacing: Theme.sm; rowSpacing: Theme.sm
        AppField { id: search; objectName: "searchField"; placeholderText: "Find"; Layout.fillWidth: true; Layout.minimumWidth: 0; onAccepted: bar.feedback=bar.editor && bar.editor.find(text,matchCase.checked,whole.checked) ? "Match selected" : "No matches"; Keys.onEscapePressed: bar.closeRequested() }
        AppField { id: replacement; objectName: "replaceField"; placeholderText: "Replace with"; Layout.fillWidth: true; Layout.minimumWidth: 0 }
        RowLayout {
            Layout.fillWidth: true; Layout.columnSpan: layout.columns === 2 ? 2 : 1
            ActionButton { text: "Next"; objectName: "findNext"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: bar.feedback=bar.editor && bar.editor.find(search.text,matchCase.checked,whole.checked) ? "Match selected" : "No matches" }
            ActionButton { text: "Replace"; objectName: "replaceOne"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: bar.feedback=bar.editor && bar.editor.replaceSelection(search.text,replacement.text,matchCase.checked,whole.checked) ? "Replaced match" : "No matches" }
            ActionButton { text: "All"; objectName: "replaceAll"; help: "Replace all matching occurrences in this document"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: bar.feedback="Replaced "+(bar.editor ? bar.editor.replaceAll(search.text,replacement.text,matchCase.checked,whole.checked) : 0)+" matches" }
        }
        RowLayout { Layout.columnSpan: layout.columns === 2 ? 2 : 1; CheckBox { id: matchCase; text: "Aa"; Accessible.name: "Case sensitive"; implicitHeight: Theme.controlHeight } CheckBox { id: whole; text: "Word"; Accessible.name: "Whole words only"; implicitHeight: Theme.controlHeight } }
        Label { text: bar.feedback; Layout.fillWidth: true; Layout.columnSpan: layout.columns-1; color: Theme.muted; wrapMode: Text.WordWrap }
        ActionButton { objectName: "closeSearch"; text: "Close"; onClicked: bar.closeRequested() }
    }
}
