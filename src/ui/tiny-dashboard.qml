import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    id: window
    width: 100
    height: 100
    x: mouseTrackerProxy.x - width / 2
    y: mouseTrackerProxy.y- height / 2
    flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    color: "#00000000"
    visible: true
    title: "Tiny dashboard"

    Center {

    }

    Connections {
        target: mouseTrackerProxy
        onMouseGlobalPositionChanged: {
            window.x = x - window.width / 2;
            window.y = y - window.height / 2;
        }
    }
}
