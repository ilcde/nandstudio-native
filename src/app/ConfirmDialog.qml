// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
AppDialog {
    id: dialog
    implicitHeight: 320 * Theme.scale
    property string message: ""
    property string actionText: "Discard changes"
    property bool showSave: false
    signal saveRequested()
    signal confirmed()
    contentItem: Flickable {
        id: scroll; contentWidth: width; contentHeight: messageText.implicitHeight; implicitHeight: Math.min(160*Theme.scale,messageText.implicitHeight); clip: true; boundsBehavior: Flickable.StopAtBounds
        Text { id: messageText; text: dialog.message; width: scroll.width; wrapMode: Text.WordWrap; color: Theme.text; font: dialog.font }
    }
    footer: RowLayout { spacing: Theme.sm; Item { Layout.fillWidth: true } ActionButton { text: "Cancel"; Layout.minimumWidth: 0; onClicked: dialog.reject(); Layout.bottomMargin: Theme.md } ActionButton { visible: dialog.showSave; text: "Save"; Layout.minimumWidth: 0; onClicked: dialog.saveRequested(); Layout.bottomMargin: Theme.md } ActionButton { text: dialog.actionText; Layout.minimumWidth: 0; onClicked: {dialog.confirmed();dialog.accept()} Layout.rightMargin: Theme.md; Layout.bottomMargin: Theme.md } }
}
