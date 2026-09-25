// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 1320; height: 860; minimumWidth: 320; minimumHeight: 280
    visible: true; title: "NandStudio"
    property bool dark: preferences.values.dark
    property bool compact: width < 980 * Theme.scale
    property int mobilePane: 0
    property int screenRevision: 0
    property bool closeApproved: false
    property int closingDocument: -1
    property string pathOperation: "create"
    property string operationPath: ""
    font.family: Qt.platform.os === "windows" ? "Segoe UI" : "sans-serif"
    font.pixelSize: Theme.bodySize
    color: Theme.canvas
    palette.window: Theme.canvas; palette.base: Theme.surface; palette.text: Theme.text
    palette.windowText: Theme.text; palette.button: Theme.raised; palette.buttonText: Theme.text
    palette.highlight: Theme.selection; palette.highlightedText: Theme.text; palette.placeholderText: Theme.muted
    onDarkChanged: { Theme.dark=dark; preferences.set("dark",dark) }
    Binding { target: Theme; property: "scale"; value: preferences.values.uiScale }
    Component.onCompleted: Theme.dark=dark
    function activeEditor() { return editorArea.activeEditor() }
    function showNewFile() { pathOperation="create"; pathDialog.title="Create file in workspace";pathDialog.actionText="Create";pathDialog.value="";pathDialog.prompt="Filename relative to the workspace. Parent folders must exist.";pathDialog.open() }
    function requestClose(index) { if(studio.documents[index].dirty){closingDocument=index;closeDocumentDialog.message="Save or discard changes to "+studio.documents[index].name+"?";closeDocumentDialog.open()}else studio.closeDocument(index) }
    onClosing: close => { if(!closeApproved && studio.hasDirtyDocuments()){close.accepted=false;exitDialog.open()} }
    onActiveChanged: if(!active){studio.key(0);studio.suspend()}
    Shortcut { sequences: [StandardKey.Save]; onActivated: if(studio.active>=0)studio.documents[studio.active].save() }
    Shortcut { sequences: [StandardKey.Open]; onActivated: openFile.open() }
    Shortcut { sequences: [StandardKey.Find]; onActivated: {if(compact)mobilePane=1;editorArea.focusSearch()} }
    Shortcut { sequence: Qt.platform.os === "osx" ? "Meta+B" : "Ctrl+B"; onActivated: studio.build() }
    Shortcut { sequence: Qt.platform.os === "osx" ? "Meta+G" : "Ctrl+G"; onActivated: lineDialog.open() }
    Shortcut { sequences: [StandardKey.Close]; onActivated: if(studio.active>=0)requestClose(studio.active) }
    Connections {
        target: studio
        function onStateChanged() { root.screenRevision++ }
        function onDiagnostic(document,line,column,message) { studio.active=document;if(root.compact)root.mobilePane=1;let p=root.activeEditor();if(p&&line>0)p.goTo(line,column) }
        function onFilesChanged() { if(studio.workspace.length){let recent=(preferences.values.recentWorkspaces || []).filter(p => p!==studio.workspace);recent.unshift(studio.workspace);preferences.set("recentWorkspaces",recent.slice(0,10))} }
        function onSearchChanged() { if(root.compact)root.mobilePane=3 }
        function onConflict(document) { conflictDialog.document=document;conflictDialog.open() }
    }
    header: AppToolbar { onFolderRequested: folder.open();onFileRequested: openFile.open();onSettingsRequested: settingsDialog.open();onSearchRequested: {if(root.compact)root.mobilePane=1;editorArea.focusSearch()} }
    FileDialog { id: openFile; title: "Open project file"; onAccepted: {studio.open(selectedFile);if(root.compact)root.mobilePane=1} }
    FolderDialog { id: folder; title: "Open workspace"; onAccepted: studio.openWorkspace(selectedFolder) }
    PathDialog {
        id: pathDialog
        onSubmitted: value => {
            let ok=root.pathOperation==="folder" ? studio.createFolder(value) : root.pathOperation==="rename" ? studio.renameFile(root.operationPath,value) : studio.createFile(value)
            if(ok){accept();if(root.compact)root.mobilePane=1}else failure=studio.lastError
        }
    }
    ConfirmDialog { id: trashDialog; objectName: "trashDialog"; title: "Move to recovery trash"; actionText: "Move"; message: "Move "+root.operationPath+" into .nandstudio-trash inside this workspace? The task console will show its recovery location."; onConfirmed: studio.trashFile(root.operationPath) }
    GoToLineDialog { id: lineDialog; onRequested: line => {let pane=root.activeEditor();if(pane)pane.goTo(line,1)} }
    SettingsDialog { id: settingsDialog }
    ConflictDialog { id: conflictDialog }
    ConfirmDialog { id: closeDocumentDialog; objectName: "closeDocumentDialog"; title: "Unsaved document"; message: ""; showSave: true; onSaveRequested: {if(studio.documents[root.closingDocument].save()){studio.closeDocument(root.closingDocument);accept()}} onConfirmed: studio.closeDocumentDiscard(root.closingDocument) }
    ConfirmDialog { id: exitDialog; objectName: "exitDialog"; title: "Unsaved changes"; message: "Some documents have unsaved changes. Save all before closing, or discard the current buffers."; showSave: true; onSaveRequested: {if(studio.saveAll()){root.closeApproved=true;root.close()}} onConfirmed: {studio.discardRecovery();root.closeApproved=true;root.close()} }
    ColumnLayout {
        anchors.fill: parent; spacing: Theme.xs
        TabBar {
            visible: root.compact; Layout.fillWidth: true; currentIndex: root.mobilePane
            onCurrentIndexChanged: root.mobilePane=currentIndex
            TabButton { text: "Files"; implicitHeight: Theme.controlHeight } TabButton { text: "Editor"; implicitHeight: Theme.controlHeight } TabButton { text: "Machine"; implicitHeight: Theme.controlHeight } TabButton { text: "Console"; implicitHeight: Theme.controlHeight }
        }
        SplitView {
            id: horizontal; Layout.fillWidth: true; Layout.fillHeight: true
            onResizingChanged: if(!resizing&&!root.compact){preferences.set("workspaceWidth",workspacePane.width);preferences.set("machineWidth",machinePane.width)}
            WorkspacePane {
                id: workspacePane
                visible: !root.compact || root.mobilePane===0
                SplitView.preferredWidth: root.compact ? root.width : preferences.values.workspaceWidth
                SplitView.minimumWidth: root.compact ? 0 : 190
                onCreateRequested: root.showNewFile()
                onFolderRequested: {root.pathOperation="folder";pathDialog.title="Create folder";pathDialog.actionText="Create";pathDialog.prompt="Folder relative to the workspace";pathDialog.value="";pathDialog.open()}
                onRenameRequested: path => {root.operationPath=path;root.pathOperation="rename";pathDialog.title="Rename file";pathDialog.actionText="Rename";pathDialog.prompt="New filename in the same folder";pathDialog.value=path.split("/").pop();pathDialog.open()}
                onTrashRequested: path => {root.operationPath=path;trashDialog.open()}
                onOpened: if(root.compact)root.mobilePane=1
            }
            SplitView {
                id: vertical; onResizingChanged: if(!resizing&&!root.compact)preferences.set("consoleHeight",consolePane.height)
                visible: !root.compact || root.mobilePane===1 || root.mobilePane===3
                orientation: Qt.Vertical; SplitView.fillWidth: true; SplitView.minimumWidth: 0
                EditorWorkspace { id: editorArea; visible: !root.compact || root.mobilePane===1; SplitView.fillHeight: true; SplitView.minimumHeight: root.compact ? 0 : 140; onCloseRequested: index => root.requestClose(index) }
                ConsolePane { id: consolePane; visible: !root.compact || root.mobilePane===3; SplitView.preferredHeight: preferences.values.consoleHeight; SplitView.minimumHeight: root.compact ? 0 : 120 }
            }
            MachinePane { id: machinePane; visible: !root.compact || root.mobilePane===2; SplitView.preferredWidth: root.compact ? root.width : preferences.values.machineWidth; SplitView.minimumWidth: root.compact ? 0 : 310; revision: root.screenRevision }
        }
    }
}
