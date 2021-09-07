import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Item {
        width: 1920
        height: 1000

        Button {
            id:b4
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top:parent.top
            anchors.topMargin: 50

            text: "Calibrate"


            onClicked: {
//                stackView.push({item: Calibration, properties: {humanType: "collaborator"}})
                stackView.push(Qt.resolvedUrl("Calibration.qml"));
            }
        }


    ColumnLayout {
        id: columnleft
        x: 160
        y:100
        width: 640
        //        anchors.right: parent.right
        //        anchors.verticalCenter: parent.verticalCenter
        height: 1000/4
        spacing: 20

        Button {
            id:b3
            text: "Start Streaming ICC"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            //REMEMBER THAT HERE YOU HAVE TO SEND THE RPC TO ANOTHER MODULE!! EMG HUMAN!
            onClicked: {
                emgUi.colStartStreamingIcc();
            }
        }
        Button {
            id:b19
            text: "Stop Streaming ICC"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            onClicked: {
                emgUi.colStopStreamingIcc();
            }
        }
        // Button {
        //     id:b30
        //     text: "Get Status"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendCol2RobotRPC("status"));
        //     }
        // }
        // Button {
        //     id:b20
        //     text: "Start Imp. Ctrl"
        //     Layout.alignment: Qt.AlignCenter
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     onClicked: {
        //         console.log(emgUi.sendCol2RobotRPC("start"));
        //     }
        // }
        // Button {
        //     id:b21
        //     height: 25
        //     text: "Stop Imp. Ctrl"
        //     Layout.alignment: Qt.AlignCenter
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     onClicked: {
        //         console.log(emgUi.sendCol2RobotRPC("stop"));
        //     }
        // }
        // Button {
        //     id:b5
        //     text: "Set LEADER"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendCol2RobotRPC("leader_behavior"));
        //     }
        // }
        // Button {
        //     id:b6
        //     text: "Set FOLLOWER"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendCol2RobotRPC("follower_behavior"));
        //     }
        // }
        // Button {
        //     id:b7
        //     text: "Set DIRECT / 3 STATES"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter

        //     onClicked: {
        //         console.log(emgUi.sendCol2RobotRPC("mixed_behavior","direct_3classes"));
        //     }
        // }
        // Button {
        //     id:b8
        //     text: "Set INVERSE / 3 STATES"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter

        //     onClicked: {
        //         console.log(emgUi.sendCol2RobotRPC("mixed_behavior","inverse_3classes"));
        //     }
        // }
    }

    ColumnLayout {
        id: columnRight
        x: 1920/2 + 160
        y:100

        width: 640
        height: 1000/4
        spacing: 20
        Button {

            text: "Start ICC Yarp"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            //REMEMBER THAT HERE YOU HAVE TO SEND THE RPC TO ANOTHER MODULE!! EMG HUMAN!
            onClicked: {
                emgUi.opStartStreamingIcc();
            }
        }
        Button {

            text: "Stop ICC Yarp"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            onClicked: {
                emgUi.opStopStreamingIcc();
            }
        }
        Button {

            text: "Start ICC ROS"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            onClicked: {
                emgUi.opStartStreamingIccRos();
            }
        }
        Button {

            text: "Stop ICC ROS"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 200
            Layout.preferredHeight: 50
            onClicked: {
                emgUi.opStopStreamingIccRos();
            }
        }
        // Button {

        //     text: "Get Status"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendOp2RobotRPC("status"));
        //     }
        // }

        // Button {
        //     id:b2
        //     text:"Start Imp. Ctrl"
        //     Layout.alignment: Qt.AlignCenter
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     onClicked: {
        //         console.log(emgUi.sendOp2RobotRPC("start"));
        //     }
        // }
        // Button {
        //     height: 25
        //     text: "Stop Imp. Ctrl"
        //     Layout.alignment: Qt.AlignCenter
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     onClicked: {
        //         console.log(emgUi.sendOp2RobotRPC("stop"));
        //     }
        // }
        // Button {
        //     id:b12
        //     text: "Set Leader"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendOp2RobotRPC("leader_behavior"));
        //     }
        // }
        // Button {
        //     id:b13
        //     text: "Set Follower"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendOp2RobotRPC("follower_behavior"));
        //     }
        // }
        // Button {
        //     id:b14
        //     text: "Set DIRECT / 3 STATES"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendOp2RobotRPC("mixed_behavior","direct_3classes"));
        //     }
        // }
        // Button {
        //     id:b15
        //     text: "Set INVERSE / 3 STATES"
        //     Layout.preferredWidth: 200
        //     Layout.preferredHeight: 50
        //     Layout.alignment: Qt.AlignCenter
        //     onClicked: {
        //         console.log(emgUi.sendOp2RobotRPC("mixed_behavior", "inverse_3classes"));
        //     }
        // }
    }
    Text{
        anchors.horizontalCenter: columnRight.horizontalCenter
        anchors.bottom: columnleft.top
        text:"Operator"
        font.pointSize: 30
        color:"white"
    }
    Text{
        width: 220
        anchors.horizontalCenter: columnleft.horizontalCenter
        anchors.bottom: columnRight.top
        text:"Collaborator"
        font.pointSize: 30
        color:"white"
    }



}
