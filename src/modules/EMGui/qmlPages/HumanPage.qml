import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Item {
        width: mainWin.width
        height: mainWin.height
//    width: 1200
//    height: 800

        Button {
            id:b4
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top:parent.top
            anchors.topMargin: 100

            text: "Calibrate"

            onClicked: {
//                stackView.push({item: Calibration, properties: {humanType: "collaborator"}})
                stackView.push(Qt.resolvedUrl("Calibration.qml"));
            }
        }


    ColumnLayout {
        id: columnRight
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width/2
        height: parent.height/4
        spacing: 20

        Button {
            id:b3
            text: "Start Streaming"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 200
        }
    }

    ColumnLayout {
        id: columnleft
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width/2
        height: parent.height/4
        spacing: 20

        Button {
            id:b2
            text: "Start Streaming"
            Layout.preferredWidth: 200
            Layout.alignment: Qt.AlignCenter
        }
    }
    Text{
        anchors.horizontalCenter: columnleft.horizontalCenter
        anchors.bottom: columnleft.top
        text:"Operator"
        font.pointSize: 30
        color:"white"
    }
    Text{
        anchors.horizontalCenter: columnRight.horizontalCenter
        anchors.bottom: columnRight.top
        text:"Collaborator"
        font.pointSize: 30
        color:"white"
    }

}
