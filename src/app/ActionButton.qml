// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls.Basic
Button {
    id: control
    property bool primary: false
    property string help: ""
    implicitWidth: Math.max(64, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Theme.controlHeight
    leftPadding: Theme.md; rightPadding: Theme.md; topPadding: Theme.sm; bottomPadding: Theme.sm
    hoverEnabled: Qt.platform.os !== "android" && Qt.platform.os !== "ios"
    Accessible.name: text
    Accessible.description: help
    ToolTip.visible: hoverEnabled && hovered && !down && help.length > 0
    ToolTip.text: help
    ToolTip.delay: 700
    contentItem: Text { text: control.text; font: control.font; color: !control.enabled ? Theme.muted : Theme.text; opacity: control.enabled ? 1 : 0.55; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; wrapMode: Text.WordWrap }
    background: Rectangle { radius: Theme.radius; color: control.down ? Theme.selection : control.hovered || control.primary ? Theme.raised : Theme.surface; border.color: control.activeFocus ? Theme.accent : Theme.border; border.width: control.activeFocus ? 2 : 1; opacity: control.enabled ? 1 : 0.6 }
}
