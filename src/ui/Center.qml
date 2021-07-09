import QtQuick 2.12
import QtQuick.Controls 2.12

Rectangle {
    width: 100
    height: 100
    color: "blue"
    radius: width * 0.5

    Text {
        anchors.centerIn: parent
        text: "Hello, World!"
    }
}
