import QtQuick 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    id: window
    width: 1024
    height: 600
    flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    color: "#00000000"
    visible: true
    title: "Tiny dashboard"

    Rectangle {
        width: 1024
        height: 600
        color: "#03e3fc"
        opacity: 0.5
    }
}
