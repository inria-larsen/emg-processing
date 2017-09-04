import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.4

Item {
    property int nCalibrations: 0
    property int sensorId: 0

    height: 30
//    width: parent.width
    width:500
    anchors.horizontalCenter: parent.horizontalCenter
    MouseArea{
        anchors.fill: parent
        onReleased: {
            selectedOpSensorIdx = sensorId;
            console.log(selectedOpSensorIdx);
        }
        Rectangle {
            width:500
            height: 3
            color: "#424246"
            anchors.bottom: calibBut.top
        }

        Text{
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text:"Sensor  : #"+sensorId
            font.pointSize: 14
            color:"white"
        }
        Rectangle{
        //this is actually a circle
            id:circle1
            width:20
            height:20
            color: (nCalibrations > 0) ?"green": "red"
            radius: width/2
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }
        Rectangle{
        //this is actually a circle
            id:circle2
            width:20
            height:20
            color: (nCalibrations >1) ?"green": "red"
            radius: width/2
            anchors.right: parent.right
            anchors.rightMargin: 50
            anchors.verticalCenter: parent.verticalCenter
        }
        Rectangle{
        //this is actually a circle
            id:circle3
            width:20
            height:20
            color: (nCalibrations >2) ?"green": "red"
            radius: width/2
            anchors.right: parent.right
            anchors.rightMargin: 100
            anchors.verticalCenter: parent.verticalCenter
        }

        Button{
            id:calibBut
            width: 150
            height: 30
            text: "Calibrate"
    //        font.pointSize: 19

            anchors.centerIn: parent
    //        style:


            onClicked: {
                if(nCalibrations != 3 && (!calibTimer.running) ) nCalibrations++;
                calibTimer.start();
            }
        }
    }

}
