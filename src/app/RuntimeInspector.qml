// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
ColumnLayout {
    property var instruction: {studio.state;studio.hardwareNeedsReload;return studio.cpuInstruction()}
    Label { text: "LIVE DEBUGGER"; font.bold: true }
    Label { visible: studio.state.mode==="CPU Emulator"; text: "ROM["+studio.state.PC+"]: "+studio.state.instructionBits; font.family: "monospace"; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
    Label { visible: studio.state.mode==="CPU Emulator"; text: instruction.decoded; font.family: "monospace"; Layout.fillWidth: true }
    ActionButton { visible: studio.state.mode==="CPU Emulator"; text: "Go to current ASM instruction"; enabled: instruction.canNavigate; Layout.fillWidth: true; onClicked: studio.navigateTo(instruction.path,instruction.line,1) }
    Label { visible: studio.state.mode==="CPU Emulator"&&instruction.sourceChanged; text: "Source differs from loaded snapshot or is closed. Reload explicitly to refresh instruction mapping."; wrapMode: Text.WordWrap; Layout.fillWidth: true }
    Label { text: studio.state.pauseReason; visible: text.length>0; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: Theme.accent }
    RowLayout {
        visible: !studio.state.hardware
        SpinBox { id: address; objectName: "breakpointAddress"; from: 0; to: 32767; editable: true; Layout.fillWidth: true }
        ActionButton { objectName: "addBreakpoint"; text: "Break at PC"; enabled: !studio.busy; onClicked: studio.setBreakpoint(address.value,true) }
    }
    Repeater {
        model: studio.state.hardware?[]:studio.state.breakpoints
        ActionButton { required property var modelData; text: "Remove breakpoint "+modelData; Layout.fillWidth: true; enabled: !studio.busy; onClicked: studio.setBreakpoint(modelData,false) }
    }
    RowLayout {
        AppField { id: watch; objectName: "watchExpression"; placeholderText: "Watch: RAM[0], D, out..."; Layout.fillWidth: true; Layout.minimumWidth: 0; onAccepted: {studio.addWatch(text);text=""} }
        ActionButton { text: "Watch"; onClicked: {studio.addWatch(watch.text);watch.text=""} }
    }
    Repeater {
        model: {studio.state;return studio.watchValues()}
        RowLayout {
            required property var modelData
            Label { text: modelData.name+": "+modelData.value; wrapMode: Text.WrapAnywhere; Layout.fillWidth: true }
            ActionButton { text: "Remove"; onClicked: studio.removeWatch(modelData.name) }
        }
    }
}
