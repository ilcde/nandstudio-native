// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    id: trace
    property var samples: studio.hardwareTrace
    property string signalName: signalChoice.currentText
    property int radix: formatChoice.currentIndex === 1 ? 16 : formatChoice.currentIndex === 2 ? 2 : 10
    Label { text: "SIGNAL HISTORY"; font.bold: true; color: Theme.muted }
    RowLayout {
        Layout.fillWidth: true
        ComboBox { id: signalChoice; objectName: "traceSignal"; Layout.fillWidth: true; Layout.minimumWidth: 0; model: studio.state.pins; textRole: "name" }
        ComboBox { id: formatChoice; objectName: "traceFormat"; Layout.preferredWidth: 95; model: ["Decimal","Hex","Binary"] }
    }
    Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: "Last 256 explicit Eval / Tick / Tock actions. Horizontal spacing represents actions, not elapsed wall time."; color: Theme.muted; font.pixelSize: 11 }
    Canvas {
        id: waveform
        objectName: "hardwareWaveform"
        Layout.fillWidth: true; Layout.preferredHeight: 92
        property var history: trace.samples
        property string signalName: trace.signalName
        onHistoryChanged: requestPaint()
        onSignalNameChanged: requestPaint()
        onWidthChanged: requestPaint()
        Connections { target: Theme; function onDarkChanged() { waveform.requestPaint() } }
        onPaint: {
            const ctx=getContext("2d");ctx.clearRect(0,0,width,height);
            ctx.fillStyle=Theme.canvas;ctx.fillRect(0,0,width,height);
            const data=history.slice(-32);if(!data.length||!signalName)return;
            ctx.strokeStyle=Theme.accent;ctx.lineWidth=2;ctx.beginPath();
            let previous=0;
            for(let i=0;i<data.length;i++){
                const value=data[i].values[signalName],x=8+i*Math.max(1,(width-16)/data.length);
                const y=value===0?67:25;
                if(i===0)ctx.moveTo(x,y);else {ctx.lineTo(x,previous);ctx.lineTo(x,y)}
                ctx.lineTo(8+(i+1)*(width-16)/data.length,y);previous=y;
            }
            ctx.stroke();ctx.fillStyle=Theme.text;ctx.font="11px monospace";
            ctx.fillText("0 / nonzero; exact bus values below",8,86);
        }
    }
    ScrollView {
        Layout.fillWidth: true; Layout.preferredHeight: 144; clip: true; contentWidth: availableWidth
        ListView {
            id: traceRows; objectName: "hardwareTraceRows"; model: trace.samples
            onCountChanged: positionViewAtEnd()
            delegate: Label {
                required property var modelData
                width: traceRows.width; elide: Text.ElideRight; font.family: "monospace"; color: Theme.text
                text: modelData.time + " " + modelData.event + "  " + trace.signalName + " = " + studio.formatWord(modelData.values[trace.signalName] || 0,trace.radix)
            }
        }
    }
    ActionButton { text: "Clear history"; objectName: "clearHardwareTrace"; Layout.fillWidth: true; onClicked: studio.clearHardwareTrace() }
}
