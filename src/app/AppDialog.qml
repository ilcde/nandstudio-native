// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
Dialog {
    id: dialog
    parent: Overlay.overlay
    modal: true; focus: true
    padding: Theme.md; margins: Theme.sm
    property real preferredWidth: 440
    implicitWidth: preferredWidth * Theme.scale
    width: Math.min(implicitWidth, Math.max(0,parent.width - Theme.md * 2))
    height: Math.min(implicitHeight, Math.max(0,parent.height - Theme.md * 2))
    contentWidth: Math.max(0,width-leftPadding-rightPadding)
    x: Math.round((parent.width-width)/2); y: Math.max(Theme.sm,Math.round((parent.height-height)/2))
    closePolicy: Popup.CloseOnEscape
    background: Rectangle { radius: Theme.radius; color: Theme.surface; border.color: Theme.border; border.width: 1 }
    header: Label { text: dialog.title; padding: Theme.md; color: Theme.text; font.pixelSize: Theme.bodySize+4; font.bold: true; wrapMode: Text.WordWrap }
}
