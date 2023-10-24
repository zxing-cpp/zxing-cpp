//
//  demo
//
// Copyright 2022 KURZ Digital Solutions GmbH
//
// SPDX-License-Identifier: Apache-2.0

import UIKit
import ZXingCpp

class WriteViewController: UIViewController {
    @IBOutlet fileprivate var imageView: UIImageView!

    // MARK: - Actions

    @IBAction func textFieldChanged(_ sender: UITextField) {
        guard let text = sender.text,
              let image = try? ZXIBarcodeWriter().write(text, format: .QR_CODE, width: 200, height: 200, margin: -1, ecLevel: QR_ERROR_CORRECTION_LOW)
        else {
            return
        }

        imageView.image = UIImage(cgImage: image.takeRetainedValue())
    }
}
