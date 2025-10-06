import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import Demo

Item {
    required property color color
    id: control

    QtObject {
        id: _
        readonly property bool isColorLight: Utils.isColorLight(control.color)
        readonly property color lightReveal: Qt.rgba(1, 1, 1, 0.784)
        readonly property color darkReveal: Qt.rgba(0, 0, 0, 0.784)
    }

    Shape {
        id: shapeReveal
        anchors {
            fill: parent
            margins: 1
        }
        asynchronous: true
        preferredRendererType: Shape.CurveRenderer
        visible: mouseArea.containsMouse
        opacity: {
            if (_.isColorLight) {
                return mouseArea.containsPress ? 0.7 : 0.5;
            } else {
                return mouseArea.containsPress ? 0.5 : 0.3;
            }
        }

        ShapePath {
            strokeWidth: -1
            strokeColor: Qt.color("transparent")
            fillGradient: RadialGradient {
                centerX: mouseArea.mouseX
                centerY: mouseArea.mouseY
                centerRadius: Math.max(control.width, control.height)
                focalX: mouseArea.mouseX
                focalY: mouseArea.mouseY

                GradientStop {
                    position: 0
                    color: _.isColorLight ? _.darkReveal : _.lightReveal
                }

                GradientStop {
                    position: 1
                    color: Qt.color("transparent")
                }
            }

            PathRectangle {
                width: shapeReveal.width
                height: shapeReveal.height
            }
        }
    }

    Shape {
        id: shapeRevealFrame
        anchors.fill: parent
        asynchronous: true
        preferredRendererType: Shape.CurveRenderer
        visible: false
        //opacity: mouseArea.containsPress ? 0.8 : 0.5

        ShapePath {
            strokeWidth: -1
            strokeColor: Qt.color("transparent")
            fillGradient: RadialGradient {
                centerX: mouseArea.mouseX
                centerY: mouseArea.mouseY
                centerRadius: Math.min(control.width, control.height)
                focalX: mouseArea.mouseX
                focalY: mouseArea.mouseY

                GradientStop {
                    position: 0
                    color: _.isColorLight ? Qt.color("black") : Qt.color("white")
                }

                GradientStop {
                    position: 1
                    color: Qt.color("transparent")
                }
            }

            PathRectangle {
                width: shapeRevealFrame.width
                height: shapeRevealFrame.height
            }
        }
    }

    Rectangle {
        id: mask
        visible: false
        layer.enabled: true
        anchors.fill: parent
        color: Qt.color("transparent")
        border {
            width: 2
            color: Qt.color("black")
        }
    }

    MultiEffect {
        id: maskEffect
        anchors.fill: parent
        blurEnabled: false
        shadowEnabled: false
        maskEnabled: true
        maskSource: mask
        source: shapeRevealFrame
        visible: mouseArea.containsMouse
        opacity: mouseArea.containsPress ? 0.8 : 0.5
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
    }
}
