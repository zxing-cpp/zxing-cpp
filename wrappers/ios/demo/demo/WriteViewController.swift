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
        let hints = ZXIEncodeHints(format: .QR_CODE, width: 200, height: 200, ecLevel: QR_ERROR_CORRECTION_LOW, margin: -1)
        guard let text = sender.text,
              let image = try? ZXIBarcodeWriter().write(text, hints: hints)
        else {
            return
        }

        imageView.image = UIImage(cgImage: image.takeRetainedValue())
    }
}
