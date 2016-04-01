import QtQuick 2.0

Rectangle {
    id: root
    implicitWidth: 100
    implicitHeight: 40
    color: "salmon"

    property alias text: textItem.text
    signal clicked

    Text {
        id: textItem
        color: "white"
        anchors.centerIn: parent
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
