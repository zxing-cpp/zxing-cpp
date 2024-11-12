import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import ZXing 1.0

ApplicationWindow {
    id: app
    visible: true
    width: 480
    height: 640
    title: Qt.application.name

    background: Rectangle {
        color: "#2e2f30"
    }

    ColumnLayout {
        id: appLayout
        anchors.fill: parent
        //Creating instance of ZXingQtQmlWriter that displays QR code
        ZXingQtQmlWriter {
            id: qrCode

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.margins: 25
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
        }

        RowLayout {
            Layout.margins: 50
            Layout.alignment: Qt.AlignTop

            TextField {
                id: codeContentsInput
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                placeholderText: "Enter contents of QR code"
                onAccepted: qrCode.generateBarcode(codeContentsInput.text)
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: "Generate"
                onClicked: qrCode.generateBarcode(codeContentsInput.text)
            }
        }
    }
}
