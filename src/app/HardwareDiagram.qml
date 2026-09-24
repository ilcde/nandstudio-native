// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: root
    property int revision: 0
    property string selectedPath: ""
    property var graph: { revision; return studio.hardwareDiagram(selectedPath) }
    Connections { target: studio; function onStateChanged() { if(root.selectedPath && !studio.state.hierarchy.some(item => item.path===root.selectedPath))root.selectedPath="" } }
    Label { text: "CHIP HIERARCHY / CONNECTIONS"; font.bold: true; color: Theme.muted }
    ComboBox { id: choice; objectName: "hierarchyChoice"; Layout.fillWidth: true; Layout.minimumWidth: 0; model: studio.state.hierarchy; textRole: "path"; displayText: root.selectedPath || studio.state.chip; onActivated: root.selectedPath=currentText }
    RowLayout {
        Label { text: "Zoom"; color: Theme.muted }
        Slider { id: zoom; Layout.fillWidth: true; from: 0.5; to: 2; value: 1; stepSize: 0.25 }
    }
    ScrollView {
        Layout.fillWidth: true; Layout.preferredHeight: 280; clip: true
        contentWidth: diagram.width; contentHeight: diagram.height
        Canvas {
            id: diagram; objectName: "hardwareDiagram"
            width: root.graph.width*zoom.value; height: root.graph.height*zoom.value
            property var graph: root.graph
            onGraphChanged: requestPaint()
            onWidthChanged: requestPaint()
            Connections { target: Theme; function onDarkChanged() { diagram.requestPaint() } }
            onPaint: {
                const ctx=getContext("2d");ctx.reset();ctx.scale(zoom.value,zoom.value);
                ctx.fillStyle=Theme.canvas;ctx.fillRect(0,0,graph.width,graph.height);
                for(const wire of graph.wires){
                    ctx.strokeStyle=wire.value===0?Theme.muted:Theme.accent;ctx.lineWidth=wire.width>1?3:1;
                    const bend=(wire.x1+wire.x2)/2;
                    ctx.beginPath();ctx.moveTo(wire.x1,wire.y1);ctx.lineTo(bend,wire.y1);ctx.lineTo(bend,wire.y2);ctx.lineTo(wire.x2,wire.y2);ctx.stroke();
                }
                for(const block of graph.blocks){
                    ctx.fillStyle=Theme.surface;ctx.fillRect(block.x,block.y,block.width,block.height);
                    ctx.strokeStyle=Theme.border;ctx.strokeRect(block.x,block.y,block.width,block.height);
                    ctx.fillStyle=Theme.text;ctx.font="bold 13px sans-serif";ctx.fillText(block.name,block.x+8,block.y+20,224);
                    ctx.font="12px monospace";
                    for(const pin of block.pins)ctx.fillText(pin.name+"["+pin.width+"] "+pin.direction+" = "+pin.value,block.x+8,pin.y+4,224);
                }
            }
        }
    }
    Label { text: "Select any hierarchy level. Lines use the loaded HDL connections; bright lines carry nonzero values."; Layout.fillWidth: true; wrapMode: Text.WordWrap; color: Theme.muted; font.pixelSize: 11 }
    ScrollView {
        Layout.fillWidth: true; Layout.preferredHeight: root.graph.wires.length ? 100 : 0; clip: true; contentWidth: availableWidth
        ListView { id: wires; model: root.graph.wires
            delegate: Label {
                required property var modelData
                width: wires.width; wrapMode: Text.WrapAnywhere; font.pixelSize: 11
                text: modelData.source+(modelData.sourceLo<0?"":"["+modelData.sourceLo+".."+(modelData.sourceLo+modelData.width-1)+"]")+" → "+modelData.target+(modelData.targetLo<0?"":"["+modelData.targetLo+".."+(modelData.targetLo+modelData.width-1)+"]")+" = "+modelData.value
            }
        }
    }
}
