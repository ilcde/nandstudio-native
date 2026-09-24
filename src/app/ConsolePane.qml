// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    padding: Theme.sm
    background: Rectangle { color: Theme.surface }
    ColumnLayout {
        anchors.fill: parent; spacing: Theme.sm
        TabBar {
            id: tabs; Layout.fillWidth: true
            TabButton { text: "Output" }
            TabButton { text: "Problems ("+studio.diagnostics.length+")" }
            TabButton { text: "Search ("+studio.searchResults.length+")" }
        }
        StackLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; currentIndex: tabs.currentIndex
            ScrollView { clip: true; TextArea { text: studio.output; readOnly: true; selectByMouse: true; color: Theme.text; font.family: "monospace"; font.pixelSize: Theme.bodySize-1; wrapMode: TextEdit.Wrap; onTextChanged: cursorPosition=length; Accessible.name: "Task output" } }
            ListView {
                clip: true; model: studio.diagnostics; ScrollBar.vertical: ScrollBar {}
                delegate: ItemDelegate { required property var modelData; width: ListView.view.width; text: modelData.message; onClicked: studio.navigateTo(modelData.path,modelData.line,modelData.column); contentItem: Label { text: parent.text; wrapMode: Text.WrapAnywhere; color: Theme.danger } }
            }
            ListView {
                clip: true; model: studio.searchResults; ScrollBar.vertical: ScrollBar {}
                delegate: ItemDelegate { required property var modelData; objectName: "searchResult_"+index; required property int index; width: ListView.view.width; text: modelData.label; onClicked: studio.navigateTo(modelData.path,modelData.line,modelData.column); contentItem: Label { text: parent.text; wrapMode: Text.WrapAnywhere; color: Theme.text } }
            }
        }
        AppField { Layout.fillWidth: true; Layout.minimumWidth: 0; placeholderText: "Command (help lists native actions)"; onAccepted: {studio.command(text);text=""} }
    }
    Connections { target: studio; function onSearchChanged(){ tabs.currentIndex=2 } function onDiagnosticsChanged(){ tabs.currentIndex=1 } }
}
