import QtQuick 2.0

Rectangle {
    implicitWidth: 100
    implicitHeight: 40
    color: "salmon"

    property alias text: textItem.text
    signal clicked

    Text {
        id: textItem
        color: "white"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: clicked()
    }
}
