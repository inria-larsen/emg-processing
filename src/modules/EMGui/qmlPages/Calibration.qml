import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import QtQuick.Controls 1.2
import "../delegates"

Item {
    width: mainWin.width
    height: mainWin.height

    property string humanType: "operator"
    property int timerCountDown: emgUi.calibDur


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
//            text: qsTr("Muscle image for human " + humanType)
//            text:emgUi.opSelectedSensor
        }
        color:"white"
    }


    ScrollView {
        width: 550
        height: 400
        highlightOnFocus: false
//        frameVisible: true
        anchors.top: muscleImg.bottom
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOn

        flickableItem.interactive: true

        ListView {
            anchors.fill: parent
            model: emgUi.opSensorIds
            delegate: CalibrationDelegate {
                sensorId:modelData
                nCalibrations: 0
            }
            spacing: 10
        }

        style: ScrollViewStyle {
            transientScrollBars: true
            handle: Item {
                implicitWidth: 30
                implicitHeight: 60
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
        ProgressBar {
            id:mvcLevel
            width: 500
            height: 50
            anchors.bottomMargin: 225
            anchors.left: muscleImg.right
            anchors.leftMargin: -200
            anchors.bottom: muscleImg.bottom
            rotation: -90
            value: emgUi.opBarLevel
            maximumValue: 0.005
        }

        Timer{
            id:calibTimer
            interval: 1 * 1000 // 100 Hz
            running: false
            repeat: true
            onTriggered: {

                timerCountDown = timerCountDown - 1;
                if(timerCountDown == 0){
                    calibTimer.stop();
                    timerCountDown = emgUi.calibDur;
                }
            }
        }

        Text {
            id: textTimerCountDown
            anchors.centerIn: muscleImg
            width: 60
            height: 110
            text: timerCountDown
            font.pixelSize: 100
            color: "red"
        }

        Timer {
            id: refreshTimer
            interval: emgUi.rate * 1000 // 100 Hz
            running: true
            repeat: true
            onTriggered: {
    //            emgScope.updateScopeData(scope1.series(0), scope1.chartName);
//                console.log("runnning timer from qml "+emgUi.rate);
                emgUi.readEmg();

            }
        }

        Button {
            id: button
            anchors.top: muscleImg.bottom
            anchors.left: muscleImg.right
            anchors.leftMargin: 25
            text: qsTr("Save Calibration")
            onClicked:{
                emgUi.opSaveCalibration();
            }
        }

}
