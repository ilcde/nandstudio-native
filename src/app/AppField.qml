// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls.Basic
TextField {
    id: control
    implicitWidth: 240 * Theme.scale
    implicitHeight: Math.max(Theme.controlHeight,contentHeight + Theme.md)
    padding: Theme.sm
    color: Theme.text; selectionColor: Theme.selection; selectedTextColor: Theme.text
    placeholderTextColor: Theme.muted; selectByMouse: true
    Accessible.name: placeholderText
    background: Rectangle { color: Theme.canvas; radius: Theme.radius; border.width: control.activeFocus ? 2 : 1; border.color: control.activeFocus ? Theme.accent : Theme.border }
}
