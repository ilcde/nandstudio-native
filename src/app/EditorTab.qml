// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Rectangle {
    id: tab
    required property var document
    property bool selected: false
    signal selectedByUser(); signal closeRequested()
    width: Math.min(240,Math.max(140,label.implicitWidth+72)); height: Theme.controlHeight
    radius: Theme.radius; color: selected ? Theme.surface : Theme.canvas; border.color: selected ? Theme.accent : Theme.border
    RowLayout {
        anchors.fill: parent; spacing: 0
        ToolButton { Layout.fillWidth: true; Layout.minimumWidth: 0; implicitHeight: Theme.controlHeight; contentItem: Label { id: label; text: tab.document.name+(tab.document.dirty ? " •" : ""); elide: Text.ElideMiddle; color: Theme.text; verticalAlignment: Text.AlignVCenter } onClicked: tab.selectedByUser(); Accessible.name: tab.document.name; ToolTip.visible: hovered; ToolTip.text: tab.document.path }
        ToolButton { text: "×"; implicitWidth: Theme.controlHeight; implicitHeight: Theme.controlHeight; onClicked: tab.closeRequested(); Accessible.name: "Close " + tab.document.name }
    }
}
