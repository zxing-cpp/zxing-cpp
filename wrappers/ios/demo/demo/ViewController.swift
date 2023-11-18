//  demo
//
// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

import UIKit
import AVFoundation
import ZXingCpp

class ViewController: UIViewController {
    let captureSession = AVCaptureSession()
    lazy var preview = AVCaptureVideoPreviewLayer(session: captureSession)
    let queue = DispatchQueue(label: "com.zxing_cpp.ios.demo")
    let reader = ZXIBarcodeReader()
    let zxingLock = DispatchSemaphore(value: 1)

    override func viewDidLoad() {
        super.viewDidLoad()

        self.preview.videoGravity = AVLayerVideoGravity.resizeAspectFill
        self.preview.frame = self.view.layer.bounds
        self.view.layer.addSublayer(self.preview)

        // setup camera session
        self.requestAccess {
            let discoverySession = AVCaptureDevice.DiscoverySession(
                deviceTypes: [
                    .builtInTripleCamera,
                    .builtInDualWideCamera,
                    .builtInDualCamera,
                    .builtInWideAngleCamera
                ],
                mediaType: .video,
                position: .back)

            let device = discoverySession.devices.first!
            let cameraInput = try! AVCaptureDeviceInput(device: device)
            self.captureSession.beginConfiguration()
            self.captureSession.addInput(cameraInput)
            let videoDataOutput = AVCaptureVideoDataOutput()
            videoDataOutput.setSampleBufferDelegate(self, queue: self.queue)
            videoDataOutput.videoSettings = [kCVPixelBufferPixelFormatTypeKey as String: kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange]
            videoDataOutput.alwaysDiscardsLateVideoFrames = true
            self.captureSession.addOutput(videoDataOutput)
            self.captureSession.commitConfiguration()
            DispatchQueue.global(qos: .background).async {
                self.captureSession.startRunning()
            }
        }
    }
}

extension ViewController {
    func requestAccess(_ completion: @escaping () -> Void) {
        if AVCaptureDevice.authorizationStatus(for: AVMediaType.video) == .notDetermined {
            AVCaptureDevice.requestAccess(for: .video) { _ in
                completion()
            }
        } else {
            completion()
        }
    }
}

extension ViewController: AVCaptureVideoDataOutputSampleBufferDelegate {
    func captureOutput(_ output: AVCaptureOutput, didOutput sampleBuffer: CMSampleBuffer, from connection: AVCaptureConnection) {
        guard self.zxingLock.wait(timeout: DispatchTime.now()) == .success else {
            // The previous image is still processed, drop the new one to prevent too much pressure
            return
        }
        let imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer)!
        if let result = try? reader.read(imageBuffer).first {
            print("Found barcode of format", result.format.rawValue, "with text", result.text)
        }
        self.zxingLock.signal()
    }
}
