import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import OcctQML 1.0
import QtQuick.Controls.Material 2.15

Rectangle {
    id: root
    width: 800
    height: 600

    // Set dark background to avoid white screen
    color: Material.theme === Material.Dark ? Material.backgroundColor : "#2E2E2E"

    // Apply Material theme
    Material.theme: Material.Dark
    Material.accent: Material.Blue

    // Add startup loading indicator
    Rectangle {
        id: loadingIndicator
        anchors.fill: parent
        color: root.color
        visible: !contentLoader.loaded
        z: 1000

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 20

            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                Material.accent: Material.Blue
                running: loadingIndicator.visible
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "Loading OCC QML Demo..."
                color: Material.primaryTextColor
                font.pixelSize: 16
            }
        }

        // Fade out animation
        Behavior on opacity {
            NumberAnimation {
                duration: 300
            }
        }

        opacity: visible ? 1.0 : 0.0
    }

    // Main Content Loader
    Loader {
        id: contentLoader
        anchors.fill: parent
        asynchronous: true

        property bool loaded: false

        sourceComponent: mainContent

        onStatusChanged: {
            if (status === Loader.Ready) {
                loaded = true;
            }
        }

        // Fade in animation
        Behavior on opacity {
            NumberAnimation {
                duration: 500
            }
        }

        opacity: loaded ? 1.0 : 0.0
    }

    // Main Content Component
    Component {
        id: mainContent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            // Provide reference for auto-display functionality
            property alias occViewerAlias: occViewer

            // Title area
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                color: Qt.darker(root.color, 1.1)
                border.color: Material.dividerColor
                border.width: 1
                radius: 8

                Text {
                    anchors.centerIn: parent
                    text: "OCC QML Demo"
                    font.pixelSize: 24
                    color: Material.primaryTextColor
                    font.bold: true
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // Left control panel
                Rectangle {
                    Layout.preferredWidth: 200
                    Layout.fillHeight: true
                    color: Qt.darker(root.color, 1.05)
                    border.color: Material.dividerColor
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
                            color: Material.primaryTextColor
                            horizontalAlignment: Text.AlignHCenter
                        }

                        // Control button group
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 35
                                text: "Toggle Window"
                                Material.background: Material.color(Material.Orange)
                                onClicked: occViewer.toggleWindow()
                            }

                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 35
                                text: "Add Test Shape"
                                Material.background: Material.color(Material.Green)
                                onClicked: occViewer.addTestShape()
                            }
                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 35
                                text: "Remove Test Shape"
                                Material.background: Material.color(Material.Red)
                                onClicked: occViewer.removeTestShape()
                            }
                            Button {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 35
                                text: "Update Test Shape"
                                Material.background: Material.color(Material.Blue)
                                onClicked: occViewer.updateTestShape()
                            }
                        }

                        // Status display
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            color: Qt.darker(root.color, 1.1)
                            border.color: Material.dividerColor
                            border.width: 1
                            radius: 4

                            Text {
                                anchors.centerIn: parent
                                text: "Status: " + (occViewer.windowVisible ? "Visible" : "Hidden")
                                font.pixelSize: 12
                                color: occViewer.windowVisible ? Material.color(Material.Green) : Material.color(Material.Red)
                                font.bold: true
                            }
                        }

                        // Fill remaining space
                        Item {
                            Layout.fillHeight: true
                        }

                        // Bottom information bar
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 30
                            color: Qt.darker(root.color, 1.1)
                            border.color: Material.dividerColor
                            border.width: 1
                            radius: 4

                            Text {
                                anchors.centerIn: parent
                                text: "OpenCASCADE QML Integration"
                                font.pixelSize: 10
                                color: Material.secondaryTextColor
                            }
                        }
                    }
                }

                // Right OCC display area
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: Qt.darker(root.color, 1.05)
                    border.color: Material.dividerColor
                    border.width: 1
                    radius: 8

                    // OCC viewer component
                    OccViewerItem {
                        id: occViewer
                        anchors.fill: parent
                        anchors.margins: 4  // Leave space for border

                        // Optimize rendering settings
                        layer.enabled: true
                        layer.smooth: true
                        layer.mipmap: true

                        // Add initialization status indicator
                        Rectangle {
                            id: occLoadingOverlay
                            anchors.fill: parent
                            color: Qt.rgba(0, 0, 0, 0.7)
                            radius: parent.parent.radius - 4
                            visible: !occViewer.windowVisible

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 10

                                BusyIndicator {
                                    Layout.alignment: Qt.AlignHCenter
                                    Material.accent: Material.Blue
                                    running: occLoadingOverlay.visible
                                    scale: 0.8
                                }

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Initializing 3D Viewer..."
                                    color: "white"
                                    font.pixelSize: 14
                                }
                            }

                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 400
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
