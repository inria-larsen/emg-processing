import QtQuick 2.0
import QtQuick.Controls 2.2

Item {
    width: mainWin.width
    height: mainWin.height
    property bool isConnectedStreaming: true
    property bool yarpRequest: false
    Button {
        id: button
        anchors.centerIn: parent
        text: isConnectedStreaming ? qsTr("Stop and Disconnect from Delsys") : qsTr("Connect to Delsys and Start")
//        height: 100
        onReleased: {
            yarpRequest = true
        }
    }

    BusyIndicator {
        id: busyIndicator
        y: button.y - 100
        anchors.horizontalCenter: button.horizontalCenter
        opacity: yarpRequest
    }

}
