// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    id: bar
    property var editor
    property alias searchField: search
    padding: Theme.sm
    implicitHeight: layout.implicitHeight + padding*2
    background: Rectangle { color: Theme.surface }
    GridLayout {
        id: layout; anchors.fill: parent; columns: bar.width < 620 ? 2 : 4; columnSpacing: Theme.sm; rowSpacing: Theme.sm
        AppField { id: search; objectName: "searchField"; placeholderText: "Find"; Layout.fillWidth: true; Layout.minimumWidth: 0; onAccepted: bar.editor && bar.editor.find(text,matchCase.checked,whole.checked) }
        AppField { id: replacement; objectName: "replaceField"; placeholderText: "Replace with"; Layout.fillWidth: true; Layout.minimumWidth: 0 }
        RowLayout {
            Layout.fillWidth: true; Layout.columnSpan: layout.columns === 2 ? 2 : 1
            ActionButton { text: "Next"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: bar.editor && bar.editor.find(search.text,matchCase.checked,whole.checked) }
            ActionButton { text: "Replace"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: bar.editor && bar.editor.replaceSelection(search.text,replacement.text,matchCase.checked,whole.checked) }
            ActionButton { text: "All"; objectName: "replaceAll"; help: "Replace all matching occurrences in this document"; Layout.fillWidth: true; Layout.minimumWidth: 0; onClicked: bar.editor && bar.editor.replaceAll(search.text,replacement.text,matchCase.checked,whole.checked) }
        }
        RowLayout { Layout.columnSpan: layout.columns === 2 ? 2 : 1; CheckBox { id: matchCase; text: "Aa"; Accessible.name: "Case sensitive"; implicitHeight: Theme.controlHeight } CheckBox { id: whole; text: "Word"; Accessible.name: "Whole words only"; implicitHeight: Theme.controlHeight } }
    }
}
