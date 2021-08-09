/*
* Copyright 2021 Axel Waggershauser
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.example.zxingcpp

import android.graphics.Bitmap
import android.graphics.ImageFormat
import android.graphics.Rect
import androidx.camera.core.ImageProxy
import java.lang.RuntimeException
import java.nio.ByteBuffer

class BarcodeReader {

    // Enumerates barcode formats known to this package.
    // Note that this has to be kept synchronized with native (C++/JNI) side.
    enum class Format {
        NONE, AZTEC, CODABAR, CODE_39, CODE_93, CODE_128, DATA_BAR, DATA_BAR_EXPANDED,
        DATA_MATRIX, EAN_8, EAN_13, ITF, MAXICODE, PDF_417, QR_CODE, UPC_A, UPC_E,
    }

    data class Options(
        val formats: Set<Format> = setOf(),
        val tryHarder: Boolean = false,
        val tryRotate: Boolean = false
    )

    data class Result(
        val format: Format = Format.NONE,
        val text: String? = null,
        val time: String? = null, // for development/debug purposes only
    )

    var options : Options = Options()

    fun read(image: ImageProxy): Result? {
        val supportedYUVFormats = arrayOf(ImageFormat.YUV_420_888, ImageFormat.YUV_422_888, ImageFormat.YUV_444_888)
        if (image.format !in supportedYUVFormats) {
            error("invalid image format")
        }

        var result = Result()
        val status = image.use {
            readYBuffer(
                it.planes[0].buffer,
                it.planes[0].rowStride,
                it.cropRect.left,
                it.cropRect.top,
                it.cropRect.width(),
                it.cropRect.height(),
                it.imageInfo.rotationDegrees,
                options.formats.joinToString(),
                options.tryHarder,
                options.tryRotate,
                result
            )
        }
        return try {
            result.copy(format = Format.valueOf(status!!))
        } catch (e: Throwable) {
            if (status == "NotFound") null else throw RuntimeException(status!!)
        }
    }

    fun read(bitmap: Bitmap, cropRect: Rect = Rect(), rotation: Int = 0): Result? {
        return read(bitmap, options, cropRect, rotation)
    }

    fun read(bitmap: Bitmap, options: Options, cropRect: Rect = Rect(), rotation: Int = 0): Result? {
        var result = Result()
        val status = with(options) {
            readBitmap(
                bitmap, cropRect.left, cropRect.top, cropRect.width(), cropRect.height(), rotation,
                formats.joinToString(), tryHarder, tryRotate, result
            )
        }
        return try {
            result.copy(format = Format.valueOf(status!!))
        } catch (e: Throwable) {
            if (status == "NotFound") null else throw RuntimeException(status!!)
        }
    }

    // setting the format enum from inside the JNI code is a hassle -> use returned String instead
    private external fun readYBuffer(
        yBuffer: ByteBuffer, rowStride: Int, left: Int, top: Int, width: Int, height: Int, rotation: Int,
        formats: String, tryHarder: Boolean, tryRotate: Boolean,
        result: Result,
    ): String?

    private external fun readBitmap(
        bitmap: Bitmap, left: Int, top: Int, width: Int, height: Int, rotation: Int,
        formats: String, tryHarder: Boolean, tryRotate: Boolean,
        result: Result,
    ): String?

    init {
        System.loadLibrary("zxing_android")
    }
}
