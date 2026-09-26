// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
AppDialog {
    id: dialog; objectName: "toolsDialog"; title: "Converters & diagnostics"; preferredWidth: 700
    standardButtons: Dialog.Close
    property var document: studio.active>=0 ? studio.documents[studio.active] : null
    property string source: document ? document.text : ""
    property bool pending: false
    function schedule() { pending=true; debounce.restart() }
    onSourceChanged: schedule()
    onDocumentChanged: schedule()
    Timer { id: debounce; interval: 600; onTriggered: if(live.checked && !studio.conversion.busy){dialog.pending=false;studio.previewConversion(bootstrap.checked)} }
    Connections { target: studio; function onConversionChanged(){if(!studio.conversion.busy&&dialog.pending)debounce.restart()} }
    contentItem: ScrollView {
        id: scroll; implicitHeight: 520; contentWidth: availableWidth; clip: true
        ColumnLayout {
            width: scroll.availableWidth; spacing: Theme.sm
            Label { text: "16-bit number converter"; font.bold: true }
            ComboBox { id: radix; model: ["Decimal", "Binary", "Hexadecimal"]; Layout.fillWidth: true }
            AppField { id: number; objectName: "converterInput"; text: "0"; placeholderText: "Value"; Layout.fillWidth: true }
            TextArea {
                objectName: "converterResult"; Layout.fillWidth: true; readOnly: true; selectByMouse: true; wrapMode: TextEdit.Wrap
                property var result: studio.convertWord(number.text,[10,2,16][radix.currentIndex])
                text: result.error || ("Binary: "+result.binary+"\nHex: "+result.hex+"\nSigned: "+result.signed+"\nUnsigned: "+result.unsigned)
            }
            Label { text: "Editor conversion preview"; font.bold: true }
            Label { text: "ASM → Hack · Jack → VM · VM folder → ASM · Hack / HDL validation"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            CheckBox { id: live; objectName: "liveDiagnostics"; text: "Live checks of the active editor buffer"; onToggled: dialog.schedule() }
            CheckBox { id: bootstrap; text: "VM bootstrap: initialize SP and call Sys.init"; visible: dialog.document!==null&&dialog.document.path.endsWith(".vm"); onToggled: dialog.schedule() }
            ActionButton { objectName: "previewConversion"; text: "Check & preview visible buffer"; enabled: dialog.document!==null&&!studio.conversion.busy; Layout.fillWidth: true; onClicked: studio.previewConversion(bootstrap.checked) }
            ActionButton { text: "Translate VM folder to ASM file"; visible: dialog.document!==null&&dialog.document.path.endsWith(".vm"); enabled: !studio.busy; Layout.fillWidth: true; onClicked: studio.build(true,bootstrap.checked) }
            Label { text: studio.conversion.status || "No preview requested"; wrapMode: Text.WordWrap; Layout.fillWidth: true; color: studio.conversion.error?Theme.danger:Theme.text }
            ActionButton { text: "Go to diagnostic"; visible: !!studio.conversion.error&&!studio.conversion.stale; Layout.fillWidth: true; onClicked: {studio.navigateTo(studio.conversion.path,studio.conversion.line,studio.conversion.column);dialog.close()} }
            TextArea { objectName: "conversionOutput"; Layout.fillWidth: true; Layout.preferredHeight: 210; readOnly: true; selectByMouse: true; font.family: "monospace"; text: studio.conversion.output || ""; wrapMode: TextEdit.NoWrap }
        }
    }
}
