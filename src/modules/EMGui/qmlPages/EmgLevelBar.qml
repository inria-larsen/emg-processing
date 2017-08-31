import QtQuick 2.0
import QtQuick.Controls 1.1

Item {
    property int stepSize: 0

    Rectangle {
        id: levelMeasure
        x: 261
        y: 121
        width: 120
        height: 250
        color: "black"
        visible: true

        Rectangle {

            id: levelBar
            anchors.bottom:parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter

            width: 90
            height: 250
            color: "orange"
            visible: true
        }
        Repeater {
            model: 20
            Rectangle {
                width: 100; height: 5
                border.width: 1
                color: "gray"
                y:modelData*5
            }
        }
    }

}
