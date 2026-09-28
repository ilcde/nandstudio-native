// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
Button {
    id: control
    property bool primary: false
    property string help: ""
    property string symbol: ""
    property bool iconOnly: false
    implicitWidth: Math.max(64, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Theme.controlHeight
    leftPadding: Theme.md; rightPadding: Theme.md; topPadding: Theme.sm; bottomPadding: Theme.sm
    hoverEnabled: Qt.platform.os !== "android" && Qt.platform.os !== "ios"
    Accessible.name: text
    Accessible.description: help
    ToolTip.visible: hoverEnabled && hovered && !down && help.length > 0
    ToolTip.text: help.length ? help : text
    ToolTip.delay: 700
    contentItem: RowLayout {
        spacing: Theme.xs; opacity: control.enabled ? 1 : 0.55
        Item { Layout.fillWidth: true }
        ActionIcon { visible: control.symbol.length>0; kind: control.symbol; ink: control.enabled ? Theme.text : Theme.muted; Layout.preferredWidth: 20; Layout.preferredHeight: 20 }
        Text { visible: !control.iconOnly; text: control.text; font: control.font; color: control.enabled ? Theme.text : Theme.muted; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.WordWrap; Layout.maximumWidth: Math.max(0,control.width-control.leftPadding-control.rightPadding-(control.symbol.length ? 24 : 0)) }
        Item { Layout.fillWidth: true }
    }
    background: Rectangle { radius: Theme.radius; color: control.down ? Theme.selection : control.hovered || control.primary ? Theme.raised : Theme.surface; border.color: control.activeFocus ? Theme.accent : Theme.border; border.width: control.activeFocus ? 2 : 1; opacity: control.enabled ? 1 : 0.6 }
}
