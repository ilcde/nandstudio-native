// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NandStudio.Native
AppDialog {
    id: dialog; objectName: "bitmapDialog"; title: "Bitmap editor"; preferredWidth: 850
    implicitHeight: 740; standardButtons: Dialog.Close
    contentItem: ScrollView {
        id: scroll; contentWidth: availableWidth; clip: true
        ColumnLayout {
            width: scroll.availableWidth; spacing: Theme.sm
            Label { text: "Draw pixels, then copy generated code into Bitmap.jack or a new ASM file."; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            RowLayout {
                SpinBox { id: columns; from: 1; to: 512; value: 48; editable: true; Layout.fillWidth: true; Accessible.name: "Bitmap width" }
                Label { text: "x" }
                SpinBox { id: rows; from: 1; to: 256; value: 32; editable: true; Layout.fillWidth: true; Accessible.name: "Bitmap height" }
                ActionButton { text: "Resize"; onClicked: canvas.resizeCanvas(columns.value,rows.value) }
            }
            Flickable {
                Layout.fillWidth: true; Layout.preferredHeight: 260; contentWidth: canvas.width; contentHeight: canvas.height; clip: true
                BitmapCanvas { id: canvas; objectName: "bitmapCanvas"; width: canvas.columns*18; height: canvas.rows*18
                    MouseArea { anchors.fill: parent; preventStealing: true
                        onPressed: mouse => canvas.beginStroke(Math.floor(mouse.x/18),Math.floor(mouse.y/18))
                        onPositionChanged: mouse => {if(pressed)canvas.stroke(Math.floor(mouse.x/18),Math.floor(mouse.y/18))}
                        onReleased: canvas.endStroke(); onCanceled: canvas.endStroke()
                    }
                }
                ScrollBar.horizontal: ScrollBar { }
                ScrollBar.vertical: ScrollBar { }
            }
            Flow { Layout.fillWidth: true; spacing: Theme.xs
                Repeater { model: ["left","right","up","down","flip","invert","rotate","clear"]
                    ActionButton { required property string modelData; text: modelData; enabled: modelData!=="rotate"||canvas.columns===canvas.rows; onClicked: canvas.transform(modelData) }
                }
                ActionButton { text: "Undo"; enabled: canvas.canUndo; onClicked: canvas.undo() }
                ActionButton { text: "Redo"; enabled: canvas.canRedo; onClicked: canvas.redo() }
            }
            Label { text: "Full canvas export. Partial final words are zero-padded. ASM starts at screen address 16384; Jack draw(location) uses a word offset. No files change automatically."; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            ComboBox { id: language; model: ["Jack", "Hack assembly"]; Layout.fillWidth: true }
            ActionButton { text: "Copy generated code"; Layout.fillWidth: true; onClicked: canvas.copyCode(language.currentIndex===1) }
            TextArea { objectName: "bitmapCode"; Layout.fillWidth: true; Layout.preferredHeight: 160; readOnly: true; selectByMouse: true; text: language.currentIndex===1?canvas.assembly:canvas.jack; font.family: "monospace" }
        }
    }
}

