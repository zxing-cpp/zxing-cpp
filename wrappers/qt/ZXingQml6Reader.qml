/*
 * Copyright 2022 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import QtMultimedia
import ZXing
import QtCore // required for CameraPermission (Qt 6.6 and above)

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
		videoSink: videoOutput.videoSink

		formats: formatCombo.selectedFormat
		tryRotate: tryRotateSwitch.checked
		tryHarder: tryHarderSwitch.checked
		tryInvert: tryInvertSwitch.checked
		tryDownscale: tryDownscaleSwitch.checked
		textMode: ZXing.TextMode.HRI

		// callback with parameter 'barcodes', called for every successfully processed frame
		onFoundBarcodes: (barcodes)=> {
			if (barcodes.length === 0)
				return

			// Store all barcode positions
			allBarcodePositions = barcodes.map(b => [b.position.topLeft, b.position.topRight, b.position.bottomRight, b.position.bottomLeft])

			// Build info text for all barcodes
			let infoParts = []
			barcodes.forEach((barcode, i) => {
				if (barcodes.length > 1)
					infoParts.push(qsTr("[%1]").arg(i + 1))
				infoParts.push(
					qsTr("Format: %1").arg(ZXingQml.FormatToString(barcode.format)),
					qsTr("Text: %1").arg(barcode.text),
					qsTr("Type: %1").arg(ZXingQml.ContentTypeToString(barcode.contentType))
				)
				if (i < barcodes.length - 1)
					infoParts.push("")
			})
			infoParts.push("", qsTr("Time: %1 ms").arg(runTime))
			info.text = infoParts.join("\n")

			resetInfo.restart()
		}

		// called for every processed frame where no barcode was detected
		onFoundNoBarcodes: ()=> {
			allBarcodePositions = []
			if (!resetInfo.running)
				info.text = "No barcode found (in %1 ms)".arg(runTime)
		}
	}

	MediaDevices {
		id: devices
	}

	// Starting from Qt 6.6, camera permission should be requested manually. For older Qt versions, CameraPermission object can be simply removed as Qt automatically requests the permission.
	CameraPermission {
		id: cameraPermission
		Component.onCompleted: {
			if (status !== Qt.PermissionStatus.Granted)
				request();
		}
	}
	// End of Qt 6.6+ section

	Camera {
		id: camera
		cameraDevice: devices.videoInputs[camerasComboBox.currentIndex] ? devices.videoInputs[camerasComboBox.currentIndex] : devices.defaultVideoInput
		focusMode: Camera.FocusModeAutoNear
		onErrorOccurred: console.log("camera error:" + errorString)
		active: cameraPermission.status === Qt.PermissionStatus.Granted // for Qt 6.6 and above
		// active: true // pre Qt 6.6
	}

	CaptureSession {
		id: captureSession
		camera: camera
		videoOutput: videoOutput
	}

	ColumnLayout {
		anchors.fill: parent

		RowLayout {
			Layout.fillWidth: true
			Layout.fillHeight: false
			visible: devices.videoInputs.length > 1
			Label {
				text: qsTr("Camera: ")
				Layout.fillWidth: false
			}
			ComboBox {
				id: camerasComboBox
				Layout.fillWidth: true
				model: devices.videoInputs
				textRole: "description"
				currentIndex: 0
			}
		}

		VideoOutput {
			id: videoOutput
			Layout.fillHeight: true
			Layout.fillWidth: true

			function mapPointToItem(point)
			{
				if (videoOutput.sourceRect.width === 0 || videoOutput.sourceRect.height === 0)
					return Qt.point(0, 0);

				let dx = point.x;
				let dy = point.y;

				if ((videoOutput.orientation % 180) == 0)
				{
					dx = dx * videoOutput.contentRect.width / videoOutput.sourceRect.width;
					dy = dy * videoOutput.contentRect.height / videoOutput.sourceRect.height;
				}
				else
				{
					dx = dx * videoOutput.contentRect.height / videoOutput.sourceRect.height;
					dy = dx * videoOutput.contentRect.width / videoOutput.sourceRect.width;
				}

				switch ((videoOutput.orientation + 360) % 360)
				{
					case 0:
					default:
						return Qt.point(videoOutput.contentRect.x + dx, videoOutput.contentRect.y + dy);
					case 90:
						return Qt.point(videoOutput.contentRect.x + dy, videoOutput.contentRect.y + videoOutput.contentRect.height - dx);
					case 180:
						return Qt.point(videoOutput.contentRect.x + videoOutput.contentRect.width - dx, videoOutput.contentRect.y + videoOutput.contentRect.height -dy);
					case 270:
						return Qt.point(videoOutput.contentRect.x + videoOutput.contentRect.width - dy, videoOutput.contentRect.y + dx);
				}
			}

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

					RowLayout {
						Label {
							text: qsTr("Formats:")
							color: "white"
						}
					ComboBox {
						id: formatCombo
						property var selectedFormat: currentIndex >= 0 ? model[currentIndex] : ZXing.None
						model: ZXingQml.ListBarcodeFormats()
						textRole: "display"
						valueRole: "display"
						currentIndex: model.indexOf(ZXing.All)
						displayText: ZXingQml.FormatToString(selectedFormat)
							delegate: ItemDelegate {
								text: ZXingQml.FormatToString(modelData)
								highlighted: formatCombo.highlightedIndex === index
							}
						}
					}
				}
			}
		}
	}
}
