import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import QtQuick.Controls 1.2
import "../delegates"

Item {
    width: mainWin.width
    height: mainWin.height

    property string humanType: "operator"

//    Image{

//    }
    Rectangle{
        id: muscleImg
        width:500
        height:500
        anchors.top:parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        Text {
            anchors.centerIn: parent
            id: textImage
            text: qsTr("Muscle image for human " + humanType)
        }
        color:"white"
    }


    ScrollView {
        width: parent.width
        height: parent.height/2
        anchors.top: muscleImg.bottom
        anchors.topMargin: 20

        flickableItem.interactive: true

        ListView {
            anchors.fill: parent
            model: 3
            delegate: CalibrationDelegate {
                sensorId:modelData
                nCalibrations: 0
            }
            spacing: 10
        }

        style: ScrollViewStyle {
            transientScrollBars: true
            handle: Item {
                implicitWidth: 14
                implicitHeight: 26
                Rectangle {
                    color: "#424246"
                    anchors.fill: parent
                    anchors.topMargin: 6
                    anchors.leftMargin: 4
                    anchors.rightMargin: 4
                    anchors.bottomMargin: 6
                }
            }
            scrollBarBackground: Item {
                implicitWidth: 14
                implicitHeight: 26
            }
        }
    }

}
