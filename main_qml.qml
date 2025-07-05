import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import GeoToys 1.0
import QtQuick.Controls.Material 2.15

Rectangle {
    id: root
    width: 800
    height: 600
    Material.theme: Material.Dark
    Material.accent: Material.Blue

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // 标题区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60

            Text {
                anchors.centerIn: parent
                text: "OCC QML Demo"
                font.pixelSize: 24
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // 左侧控制面板
            Rectangle {
                Layout.preferredWidth: 200
                Layout.fillHeight: true
                border.width: 1
                radius: 8

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 15
                    spacing: 15

                    Text {
                        Layout.fillWidth: true
                        text: "Control Panel"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // 控制按钮组
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            text: "Show Window"
                            onClicked: qmlViewer.showWindow()
                        }

                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            text: "Hide Window"
                            onClicked: qmlViewer.hideWindow()
                        }

                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 35
                            text: "Toggle Window"
                            onClicked: qmlViewer.toggleWindow()
                        }
                    }

                    // 状态显示
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        border.width: 1
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: "Status: " + (qmlViewer.windowVisible ? "Show" : "Hide")
                            font.pixelSize: 12
                        }
                    }

                    // 填充剩余空间
                    Item {
                        Layout.fillHeight: true
                    }

                    // 底部信息栏
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        border.width: 1
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: "Info: OCC QML demo"
                            font.pixelSize: 10
                        }
                    }
                }
            }

            // 右侧QmlViewer显示区域
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // QmlViewer组件 - 自适应填充整个区域
                OccQuickItem {
                    id: qmlViewer
                    anchors.fill: parent
                    anchors.margins: 2  // 留出边框空间
                    windowTitle: "Adaptive Window"
                }
            }
        }
    }
}
