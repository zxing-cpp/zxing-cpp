/*
 * Copyright 2020 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQuick.Shapes 1.12
import QtMultimedia 5.12
import ZXing 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: Qt.application.name

    property var nullPoints: [Qt.point(0,0), Qt.point(0,0), Qt.point(0,0), Qt.point(0,0)]
    property var points: nullPoints

    Timer {
        id: resetInfo
        interval: 2000
    }

    BarcodeReader {
        id: barcodeReader

        formats: (linearSwitch.checked ? (ZXing.LinearCodes) : ZXing.None) | (matrixSwitch.checked ? (ZXing.MatrixCodes) : ZXing.None)
        tryRotate: tryRotateSwitch.checked
        tryHarder: tryHarderSwitch.checked
        tryDownscale: tryDownscaleSwitch.checked

        // callback with parameter 'result', called for every successfully processed frame
        // onFoundBarcode: {}

        // callback with parameter 'result', called for every processed frame
        onNewResult: {
            points = result.isValid
                    ? [result.position.topLeft, result.position.topRight, result.position.bottomRight, result.position.bottomLeft]
                    : nullPoints

            if (result.isValid)
                resetInfo.restart()

            if (result.isValid || !resetInfo.running)
                info.text = qsTr("Format: \t %1 \nText: \t %2 \nError: \t %3 \nTime: \t %4 ms").arg(result.formatName).arg(result.text).arg(result.status).arg(result.runTime)

//            console.log(result)
        }
    }

    Camera {
        id: camera

        captureMode: Camera.CaptureViewfinder
        deviceId: QtMultimedia.availableCameras[camerasComboBox.currentIndex] ? QtMultimedia.availableCameras[camerasComboBox.currentIndex].deviceId : ""

        onDeviceIdChanged: {
            focus.focusMode = CameraFocus.FocusContinuous
            focus.focusPointMode = CameraFocus.FocusPointAuto
        }

        onError: console.log("camera error:" + errorString)
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: false
            visible: QtMultimedia.availableCameras.length > 1
            Label {
                text: qsTr("Camera: ")
                Layout.fillWidth: false
            }
            ComboBox {
                id: camerasComboBox
                Layout.fillWidth: true
                model: QtMultimedia.availableCameras
                textRole: "displayName"
                currentIndex: 0
            }
        }

        VideoOutput {
            id: videoOutput
            Layout.fillHeight: true
            Layout.fillWidth: true
            filters: [barcodeReader]
            source: camera
            autoOrientation: true

            Shape {
                id: polygon
                anchors.fill: parent
                visible: points.length == 4
                ShapePath {
                    strokeWidth: 3
                    strokeColor: "red"
                    strokeStyle: ShapePath.SolidLine
                    fillColor: "transparent"
                    //TODO: really? I don't know qml...
                    startX: videoOutput.mapPointToItem(points[3]).x
                    startY: videoOutput.mapPointToItem(points[3]).y
                    PathLine {
                        x: videoOutput.mapPointToItem(points[0]).x
                        y: videoOutput.mapPointToItem(points[0]).y
                    }
                    PathLine {
                        x: videoOutput.mapPointToItem(points[1]).x
                        y: videoOutput.mapPointToItem(points[1]).y
                    }
                    PathLine {
                        x: videoOutput.mapPointToItem(points[2]).x
                        y: videoOutput.mapPointToItem(points[2]).y
                    }
                    PathLine {
                        x: videoOutput.mapPointToItem(points[3]).x
                        y: videoOutput.mapPointToItem(points[3]).y
                    }
                }
            }

            Label {
                id: info
                color: "white"
                padding: 10
                background: Rectangle { color: "#80808080" }
            }

            ColumnLayout {
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                Switch {id: tryRotateSwitch; text: qsTr("Try Rotate"); checked: true }
                Switch {id: tryHarderSwitch; text: qsTr("Try Harder"); checked: true }
                Switch {id: tryDownscaleSwitch; text: qsTr("Try Downscale"); checked: true }
                Switch {id: linearSwitch; text: qsTr("Linear Codes"); checked: true }
                Switch {id: matrixSwitch; text: qsTr("Matrix Codes"); checked: true }
            }
        }
    }
}
