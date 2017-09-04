import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import "qmlPages"
import "delegates"

ApplicationWindow {
    id: mainWin
    visible: true
    width: 800
    height: 1000

    property int selectedOpSensorIdx: 1
    property double opBarLevel: 0.0025

    Rectangle {
        color: "#212126"
        anchors.fill: parent
    }

    toolBar: Rectangle {
//        border.bottom: 8
//        source: "images/toolbar.png"
        color: "black"
        width: parent.width
        height: 100

        Rectangle {
            id: backButton
            width: opacity ? 60 : 0
            anchors.left: parent.left
            anchors.leftMargin: 20
            opacity: stackView.depth > 1 ? 1 : 0
            anchors.verticalCenter: parent.verticalCenter
            antialiasing: true
            height: 60
            radius: 4
            color: backmouse.pressed ? "#222" : "transparent"
            Behavior on opacity { NumberAnimation{} }
//            Image {
//                anchors.verticalCenter: parent.verticalCenter
//                source: "images/navigation_previous_item.png"
//            }
            Text {
                id: backText
                text: qsTr("BACK")
                font.pointSize: 14
                color: "white"
                anchors.centerIn: parent
                style: Text.Sunken
                styleColor: "black"
            }
            MouseArea {
                id: backmouse
                anchors.fill: parent
                anchors.margins: -10
                onReleased: {

                    stackView.pop();

                }
            }
        }

        Text {
            font.pixelSize: 42
            Behavior on x { NumberAnimation{ easing.type: Easing.OutCubic} }
            //x: backButton.x + backButton.width + 20
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
            text: "EMGui"
        }
    }

    ListModel {
        id: pageModel
        ListElement {
            title: "EMG Server"
            page: "qmlPages/ServerPage.qml"
        }
        ListElement {
            title: "EMG Human"
            page: "qmlPages/HumanPage.qml"
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        // Implements back key navigation
        focus: true
        Keys.onReleased: if (event.key === Qt.Key_Back && stackView.depth > 1) {
                             stackView.pop();
                             event.accepted = true;
                         }

        initialItem: Item {
            width: parent.width
            height: parent.height
            ListView {
                model: pageModel
                anchors.fill: parent
                delegate: MenuDelegate {
                    text: title
                    onClicked: stackView.push(Qt.resolvedUrl(page))
                }
            }
        }
    }

    Timer {
        id: refreshTimer
        interval: 1* 1000 // 100 Hz
        running: true
        repeat: true
        onTriggered: {
//            emgScope.updateScopeData(scope1.series(0), scope1.chartName);
//            console.log("runnning timer from qml "+emgUi.rate);

        }
    }
}
