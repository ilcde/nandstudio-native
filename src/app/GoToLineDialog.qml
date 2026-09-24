// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
AppDialog {
    id: dialog; objectName: "lineDialog"; title: "Go to line"
    signal requested(int line)
    onOpened: { line.forceActiveFocus(); line.selectAll() }
    contentItem: AppField { id: line; objectName: "lineField"; placeholderText: "Line number"; inputMethodHints: Qt.ImhDigitsOnly; validator: IntValidator { bottom: 1; top: 10000000 } onAccepted: if(acceptableInput){dialog.requested(parseInt(text));dialog.accept()} }
    footer: RowLayout { Item { Layout.fillWidth: true } ActionButton { text: "Cancel"; onClicked: dialog.reject(); Layout.bottomMargin: Theme.md } ActionButton { text: "Go"; enabled: line.acceptableInput; primary: true; onClicked: {dialog.requested(parseInt(line.text));dialog.accept()} Layout.rightMargin: Theme.md; Layout.bottomMargin: Theme.md } }
}
