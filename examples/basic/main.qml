import QtQuick 2.6
import QtQuick.Layouts 1.3
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

    Shortcut {
        sequence: StandardKey.Undo
        onActivated: undoStack.undo()
    }

    Shortcut {
        sequence: StandardKey.Redo
        onActivated: undoStack.redo()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            Button {
                text: "New Square"
                onClicked: undoStack.addItem(canvas, squareComponent)
            }

            Button {
                text: "New Circle"
                onClicked: undoStack.addItem(canvas, circleComponent)
            }
        }

        Item {
            id: canvas

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
