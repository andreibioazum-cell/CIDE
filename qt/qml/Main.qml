import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtWebView
import CIDE 1.0

ApplicationWindow {
    id: window
    width: 420; height: 840; visible: true
    title: "CIDE"; color: "#121212"
    property url siteUrl: "https://acode.app"
    EditorController { id: controller }

    header: ToolBar {
        RowLayout { anchors.fill: parent; anchors.margins: 8
            ToolButton { text: "☰"; onClicked: drawer.open() }
            Label { text: controller.currentFile.length ? controller.currentFile.split("/").pop() : "CIDE"; color: "white"; Layout.fillWidth: true; elide: Text.ElideMiddle }
            ToolButton { text: "＋"; onClicked: editor.text = "" }
            ToolButton { text: "💾"; onClicked: controller.saveFile(controller.currentFile, editor.text) }
        }
        background: Rectangle { color: "#202124" }
    }

    Drawer { id: drawer; width: Math.min(window.width * .82, 340); height: window.height
        background: Rectangle { color: "#1b1b1b" }
        ColumnLayout { anchors.fill: parent; anchors.margins: 18; spacing: 12
            Label { text: "CIDE · Файлы"; color: "white"; font.pixelSize: 20; Layout.fillWidth: true }
            Button { text: "Открыть файл"; Layout.fillWidth: true; onClicked: picker.open() }
            Button { text: "Сайт"; Layout.fillWidth: true; onClicked: { drawer.close(); webView.visible = true; editor.visible = false } }
            Button { text: "Редактор"; Layout.fillWidth: true; onClicked: { drawer.close(); webView.visible = false; editor.visible = true } }
            Item { Layout.fillHeight: true }
            Label { text: "Qt 6 · Android"; color: "#888" }
        }
    }
    FileDialog { id: picker; title: "Открыть файл"; onAccepted: { editor.text = controller.openFile(selectedFile); webView.visible = false; editor.visible = true } }
    TextArea { id: editor; anchors.fill: parent; anchors.margins: 4; visible: true; color: "#e6e6e6"; selectionColor: "#385d83"; font.family: "monospace"; font.pixelSize: 14; wrapMode: TextEdit.NoWrap; background: Rectangle { color: "#121212" } }
    WebView { id: webView; anchors.fill: parent; visible: false; url: window.siteUrl }
}
