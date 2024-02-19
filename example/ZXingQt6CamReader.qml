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

Window {
	visible: true
	width: 640
	height: 480
	title: Qt.application.name

	property var nullPoints: [Qt.point(0,0), Qt.point(0,0), Qt.point(0,0), Qt.point(0,0)]
	property var points: nullPoints

	Timer {
		id: resetInfo
		interval: 1000
	}

	BarcodeReader {
		id: barcodeReader
		videoSink: videoOutput.videoSink

		formats: (linearSwitch.checked ? (ZXing.LinearCodes) : ZXing.None) | (matrixSwitch.checked ? (ZXing.MatrixCodes) : ZXing.None)
		tryRotate: tryRotateSwitch.checked
		tryHarder: tryHarderSwitch.checked
		tryInvert: tryInvertSwitch.checked
		tryDownscale: tryDownscaleSwitch.checked
		textMode: ZXing.TextMode.HRI

		// callback with parameter 'barcode', called for every successfully processed frame
		onFoundBarcode: (barcode)=> {
			points = [barcode.position.topLeft, barcode.position.topRight, barcode.position.bottomRight, barcode.position.bottomLeft]
			info.text = qsTr("Format: \t %1 \nText: \t %2 \nType: \t %3 \nTime: \t %4 ms").arg(barcode.formatName).arg(barcode.text).arg(barcode.contentTypeName).arg(runTime)

			resetInfo.restart()
//			console.log(barcode)
		}

		// called for every processed frame where no barcode was detected
		onFailedRead: ()=> {
			points = nullPoints

			if (!resetInfo.running)
				info.text = "No barcode found (in %1 ms)".arg(runTime)
		}
	}

	MediaDevices {
		id: devices
	}

	Camera {
		id: camera
		cameraDevice: devices.videoInputs[camerasComboBox.currentIndex] ? devices.videoInputs[camerasComboBox.currentIndex] : devices.defaultVideoInput
		focusMode: Camera.FocusModeAutoNear
		onErrorOccurred: console.log("camera error:" + errorString)
		active: true
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

			Shape {
				id: polygon
				anchors.fill: parent
				visible: points.length === 4

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
				Switch {id: tryInvertSwitch; text: qsTr("Try Invert"); checked: true }
				Switch {id: tryDownscaleSwitch; text: qsTr("Try Downscale"); checked: true }
				Switch {id: linearSwitch; text: qsTr("Linear Codes"); checked: true }
				Switch {id: matrixSwitch; text: qsTr("Matrix Codes"); checked: true }
			}
		}
	}
}
