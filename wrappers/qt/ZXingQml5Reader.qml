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

	property var allBarcodePositions: [] // Array of position arrays

	Timer {
		id: resetInfo
		interval: 1000
	}

	BarcodeReader {
		id: barcodeReader

		formats: (linearSwitch.checked ? (ZXing.LinearCodes) : ZXing.None) | (matrixSwitch.checked ? (ZXing.MatrixCodes) : ZXing.None)
		tryRotate: tryRotateSwitch.checked
		tryHarder: tryHarderSwitch.checked
		tryInvert: tryInvertSwitch.checked
		tryDownscale: tryDownscaleSwitch.checked
		textMode: ZXing.HRI

		// callback with parameter 'barcodes', called for every successfully processed frame
		onFoundBarcodes: (barcodes)=> {
			if (barcodes.length === 0)
				return

			// Store all barcode positions
			allBarcodePositions = barcodes.map(function(b) {
				return [b.position.topLeft, b.position.topRight, b.position.bottomRight, b.position.bottomLeft]
			})

			// Build info text for all barcodes
			var infoParts = []
			for (var i = 0; i < barcodes.length; i++) {
				var barcode = barcodes[i]
				if (barcodes.length > 1)
					infoParts.push(qsTr("[%1]").arg(i + 1))
				infoParts.push(
					qsTr("Format: %1").arg(barcode.formatName),
					qsTr("Text: %1").arg(barcode.text),
					qsTr("Type: %1").arg(barcode.contentTypeName)
				)
				if (i < barcodes.length - 1)
					infoParts.push("")
			}
			infoParts.push("", qsTr("Time: %1 ms").arg(runTime))
			info.text = infoParts.join("\n")

			resetInfo.restart()
		}
				infoParts.push(qsTr("Time: %1 ms").arg(runTime));
				info.text = infoParts.join("\n");

				resetInfo.restart()
//				console.log(barcodes)
			}
		}

		// called for every processed frame where no barcode was detected
		onFoundNoBarcodes: ()=> {
			allBarcodePositions = []
			if (!resetInfo.running)
				info.text = "No barcode found (in %1 ms)".arg(runTime)
		}
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

			Repeater {
				model: allBarcodePositions

				Shape {
					anchors.fill: parent
					property var points: modelData
					visible: points && points.length === 4

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
			}

			Label {
				id: info
				color: "white"
				padding: 10
				background: Rectangle { color: "#80808080" }
			}

			Rectangle {
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				color: "#80808080"
				width: controlsLayout.implicitWidth + 10
				height: controlsLayout.implicitHeight + 10

				ColumnLayout {
					id: controlsLayout
					anchors.centerIn: parent

					Switch {id: tryRotateSwitch; text: qsTr("Try Rotate"); checked: true }
					Switch {id: tryHarderSwitch; text: qsTr("Try Harder"); checked: true }
					Switch {id: tryInvertSwitch; text: qsTr("Try Invert"); checked: true }
					Switch {id: tryDownscaleSwitch; text: qsTr("Try Downscale"); checked: true }
					Switch {id: linearSwitch; text: qsTr("Linear Codes"); checked: true }
					Switch {id: matrixSwitch; text: qsTr("Matrix Codes"); checked: true }
				}
			}
		}
	}
}
