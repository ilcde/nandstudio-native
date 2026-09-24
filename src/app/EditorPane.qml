// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NandStudio.Native 1.0
Item {
    id: pane
    required property var document
    property alias editor: edit
    function goTo(line,column) { let parts=edit.text.split("\n"),pos=0; for(let i=0;i<Math.min(parts.length,line-1);++i)pos+=parts[i].length+1;edit.cursorPosition=Math.min(edit.length,pos+Math.max(0,column-1));edit.forceActiveFocus() }
    function find(query,sensitive,word) { let m=editorTools.find(edit.text,query,edit.selectionEnd,!!sensitive,!!word);if(m.start>=0){edit.select(m.start,m.end);edit.forceActiveFocus()} }
    function replaceSelection(query,replacement,sensitive,word) { if(edit.selectedText.length && (sensitive ? edit.selectedText===query : edit.selectedText.toLowerCase()===query.toLowerCase())){let p=edit.selectionStart;edit.remove(p,edit.selectionEnd);edit.insert(p,replacement)}else find(query,sensitive,word) }
    function replaceAll(query,replacement,sensitive,word) { editorTools.replaceAll(edit.textDocument,query,replacement,!!sensitive,!!word) }
    function applySelection(range) { if(range.start!==undefined){edit.select(range.start,range.end);edit.forceActiveFocus()} }
    Rectangle { anchors.fill: parent; color: Theme.canvas }
    LineNumberGutter {
        id: gutter; objectName: "gutter_"+pane.document.name
        width: Math.max(48,String(edit.lineCount).length*edit.font.pixelSize+20); anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: status.top
        document: edit.textDocument; textFont: edit.font; textColor: Theme.muted; scrollY: flick.contentY; topPadding: edit.topPadding
        Accessible.role: Accessible.StaticText; Accessible.name: "Line numbers"
    }
    Flickable {
        id: flick; anchors.left: gutter.right; anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: status.top
        clip: true; boundsBehavior: Flickable.StopAtBounds
        contentWidth: edit.width; contentHeight: edit.height
        ScrollBar.vertical: ScrollBar {}
        ScrollBar.horizontal: ScrollBar {}
        TextArea {
            id: edit; objectName: "editor_"+pane.document.name
            width: preferences.values.wordWrap ? flick.width : Math.max(flick.width,implicitWidth)
            height: Math.max(flick.height,implicitHeight)
            text: pane.document.text; color: Theme.text; selectionColor: Theme.selection; selectedTextColor: Theme.text
            font.family: preferences.values.fontFamily; font.pixelSize: preferences.values.fontSize
            selectByMouse: true; persistentSelection: true; textFormat: TextEdit.PlainText
            wrapMode: preferences.values.wordWrap ? TextEdit.Wrap : TextEdit.NoWrap
            leftPadding: Theme.sm; rightPadding: Theme.md; topPadding: Theme.sm; bottomPadding: Theme.lg
            property int matching: editorTools.matchingBracket(text,cursorPosition)
            background: Rectangle {
                color: Theme.canvas
                Rectangle { x: 0; y: edit.cursorRectangle.y; width: edit.width; height: edit.cursorRectangle.height; color: Theme.currentLine }
                Rectangle { visible: edit.matching >= 0; property rect location: edit.positionToRectangle(Math.max(0,edit.matching)); x: location.x; y: location.y; width: Math.max(edit.font.pixelSize*0.6,location.width); height: location.height; color: Theme.selection; border.color: Theme.accent }
            }
            onTextChanged: if(pane.document.text!==text)pane.document.text=text
            Component.onCompleted: pane.document.highlight(textDocument)
            onCursorRectangleChanged: { if(cursorRectangle.y<flick.contentY)flick.contentY=cursorRectangle.y;else if(cursorRectangle.y+cursorRectangle.height>flick.contentY+flick.height)flick.contentY=cursorRectangle.y+cursorRectangle.height-flick.height }
            Keys.onPressed: event => {
                if(edit.inputMethodComposing)return
                if(event.key===Qt.Key_Tab || event.key===Qt.Key_Backtab){pane.applySelection(editorTools.indent(edit.textDocument,edit.selectionStart,edit.selectionEnd,preferences.values.indentWidth,preferences.values.indentTabs,event.key===Qt.Key_Backtab || !!(event.modifiers&Qt.ShiftModifier)));event.accepted=true}
                else if(event.key===Qt.Key_Return && !(event.modifiers&(Qt.ControlModifier|Qt.MetaModifier))){if(edit.selectionEnd>edit.selectionStart)edit.remove(edit.selectionStart,edit.selectionEnd);edit.cursorPosition=editorTools.newline(edit.textDocument,edit.cursorPosition,preferences.values.indentWidth,preferences.values.indentTabs);event.accepted=true}
                else if(event.key===Qt.Key_Slash && (event.modifiers & (Qt.platform.os === "osx" ? Qt.MetaModifier : Qt.ControlModifier))){pane.applySelection(editorTools.comment(edit.textDocument,edit.selectionStart,edit.selectionEnd));event.accepted=true}
            }
            Accessible.name: pane.document.name + " editor"
        }
    }
    Label { id: status; anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right; height: Theme.lg; text: "  Ln "+edit.text.slice(0,edit.cursorPosition).split("\n").length+"   UTF-8"; color: Theme.muted; font.pixelSize: Theme.bodySize-2; verticalAlignment: Text.AlignVCenter; background: Rectangle { color: Theme.surface } }
}
