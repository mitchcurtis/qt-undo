import QtQuick 2.6
import QtQuick.Window 2.2

Window {
    width: 600
    height: 400
    visible: true

    Component {
        id: squareComponent

        Rectangle {
            color: "steelblue"
            width: 100
            height: width
        }
    }

    Component {
        id: circleComponent

        Rectangle {
            color: "darkorange"
            width: 100
            height: width
            radius: width / 2
        }
    }

    Row {
        Button {
            text: "New Square"
            onClicked: undoStack.pushItem(squareComponent)
        }

        Button {
            text: "New Circle"
            onClicked: undoStack.pushItem(circleComponent)
        }
    }
}
