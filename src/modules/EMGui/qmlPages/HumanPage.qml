import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Item {
        width: mainWin.width
        height: mainWin.height
//    width: 1200
//    height: 800

    ColumnLayout {
        id: columnRight
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width/2
        height: parent.height/4
        spacing: 20

        Button {
            id:b4
            Layout.preferredWidth: 200
            text: "Calibrate"
            Layout.alignment: Qt.AlignCenter
            onClicked: {
//                stackView.push({item: Calibration, properties: {humanType: "collaborator"}})
                stackView.push(Qt.resolvedUrl("Calibration.qml"));
            }
        }

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
            id:b1
            text: "Calibrate"
            Layout.preferredWidth: 200
            Layout.alignment: Qt.AlignCenter
            onClicked: {
//                stackView.push({item: Calibration, properties: {humanType: "operator"}})
                stackView.push(Qt.resolvedUrl("Calibration.qml"));
            }
        }

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
