// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls.Basic
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
    ToolTip.visible: hoverEnabled && hovered && !down && (help.length > 0 || iconOnly)
    ToolTip.text: help.length ? help : text
    ToolTip.delay: 700
    contentItem: Item {
        id: contents
        readonly property real symbolWidth: control.symbol.length ? 20 : 0
        readonly property real gap: symbolWidth && !control.iconOnly ? Theme.xs : 0
        implicitWidth: symbolWidth+gap+(control.iconOnly ? 0 : caption.implicitWidth)
        implicitHeight: Math.max(symbolWidth,control.iconOnly ? 0 : caption.implicitHeight)
        opacity: control.enabled ? 1 : 0.55
        ActionIcon { visible: contents.symbolWidth>0; kind: control.symbol; ink: control.enabled ? Theme.text : Theme.muted; width: 20; height: 20; anchors.verticalCenter: parent.verticalCenter; x: control.iconOnly ? (parent.width-width)/2 : 0 }
        Text { id: caption; visible: !control.iconOnly; x: contents.symbolWidth+contents.gap; width: Math.max(0,parent.width-x); height: parent.height; text: control.text; font: control.font; color: control.enabled ? Theme.text : Theme.muted; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.WordWrap }
    }
    background: Rectangle { radius: Theme.radius; color: control.down ? Theme.selection : control.hovered || control.primary ? Theme.raised : Theme.surface; border.color: control.activeFocus ? Theme.accent : Theme.border; border.width: control.activeFocus ? 2 : 1; opacity: control.enabled ? 1 : 0.6 }
}
