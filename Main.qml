import QtQuick
import QtQuick.Layouts
import Demo

Window {
    width: 600
    height: 600
    visible: true
    title: qsTr("Reveal Demo [Qt Quick Edition]")

    GridLayout {
        anchors {
            fill: parent
            margins: 50
        }
        rows: 5
        columns: 5
        rowSpacing: 1
        columnSpacing: 1

        Repeater {
            model: 25

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Utils.generateRandomColor()

                RevealRectangle {
                    anchors.fill: parent
                    color: parent.color
                }
            }
        }
    }
}
