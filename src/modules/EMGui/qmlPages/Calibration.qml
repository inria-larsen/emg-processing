import QtQuick 2.0
import QtQuick.Controls.Styles 1.1
import QtQuick.Controls 1.2
import "../delegates"

Item {
    width: mainWin.width
    height: mainWin.height

//    property string humanType: "operator"
    property int opTimerCountDown: emgUi.calibDur
    property int colTimerCountDown: emgUi.calibDur

    property bool colRest: false


//    Image{

//    }

//===========
//OP ELEMENTS
//===========
    Rectangle{
        id: muscleImg
        width:500
        height:500
        anchors.top:parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 350
        color:"white"
    }
    ProgressBar {
        id:mvcLevel
        width: 500
        height: 50
        anchors.bottomMargin: 225
        anchors.left: muscleImg.right
        anchors.leftMargin: -160
        anchors.bottom: muscleImg.bottom
        rotation: -90
        value: emgUi.opBarLevel
        maximumValue: 0.0025
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

    ScrollView {
        id:opSView
        width: 550
        height: 400
        highlightOnFocus: false
//        frameVisible: true
        anchors.top: muscleImg.bottom
        anchors.topMargin: 20
        anchors.horizontalCenter: muscleImg.horizontalCenter
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

    Timer{
        id:opCalibTimer
        interval: 1 * 1000 // 100 Hz
        running: false
        repeat: true
        onTriggered: {

            opTimerCountDown = opTimerCountDown - 1;
            if(opTimerCountDown == 0){
                opCalibTimer.stop();
                opTimerCountDown = emgUi.calibDur;
            }
        }
    }

    Text {
        id: textOpTimerCountDown
        anchors.centerIn: muscleImg
        width: 60
        height: 110
        text: opTimerCountDown
        font.pixelSize: 100
        color: "red"
    }

    //===========
    //COL ELEMENTS
    //===========

    Rectangle{
        id: colMuscleImg
        width:500
        height:500
        anchors.top:parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -350
        color:"white"
    }
    ProgressBar {
        id:colMvcLevel
        width: 500
        height: 50
        anchors.bottomMargin: 225
        anchors.left: colMuscleImg.right
        anchors.leftMargin: -160
        anchors.bottom: colMuscleImg.bottom
        rotation: -90
        value: emgUi.colBarLevel
        maximumValue: 0.0025
    }
    Button {
        id: colButton
        anchors.top: colMuscleImg.bottom
        anchors.left: colMuscleImg.right
        anchors.leftMargin: 25
        text: qsTr("Save Calibration")
        onClicked:{
            emgUi.colSaveCalibration();
        }
    }

    ScrollView {
        id:colSView
        width: 550
        height: 400
        highlightOnFocus: false
//        frameVisible: true
        anchors.top: colMuscleImg.bottom
        anchors.topMargin: 20
        anchors.horizontalCenter: colMuscleImg.horizontalCenter
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOn

        flickableItem.interactive: true

        ListView {
            anchors.fill: parent
            model: emgUi.colSensorIds
            delegate: CalibrationDelegate {
                sensorId:modelData
                nCalibrations: 0
                operator: false
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

    Timer{
        id:colRestTimer
        interval: 60*1000
        running: false
        repeat: false
        onTriggered: {
            colRest = false;
        }
    }

    Timer{
        id:colCalibTimer
        interval: 1 * 1000 // 100 Hz
        running: false
        repeat: true
        onTriggered: {

            colTimerCountDown = colTimerCountDown - 1;
            if(colTimerCountDown == 0){
                colCalibTimer.stop();
                colTimerCountDown = emgUi.calibDur;
                colRest = true;
                colRestTimer.start();
            }
        }
    }

    Text {
        id: textColTimerCountDown
        anchors.centerIn: colMuscleImg
        width: 60
        height: 110
        text: colTimerCountDown
        font.pixelSize: 100
        color: "red"
    }





    //====================
    //COMMON REFRESH TIMER
    //====================

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



}
