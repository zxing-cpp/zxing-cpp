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

package com.example.zxingcppdemo

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.*
import android.hardware.camera2.CaptureRequest
import android.media.AudioManager
import android.media.MediaActionSound
import android.media.ToneGenerator
import android.os.Bundle
import android.os.Environment
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.camera2.interop.Camera2CameraControl
import androidx.camera.camera2.interop.CaptureRequestOptions
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.graphics.toPointF
import androidx.lifecycle.LifecycleOwner
import com.example.zxingcppdemo.databinding.ActivityCameraBinding
import com.google.zxing.*
import com.google.zxing.common.HybridBinarizer
import com.zxingcpp.BarcodeReader
import com.zxingcpp.BarcodeReader.Format
import java.io.ByteArrayOutputStream
import java.io.File
import java.util.concurrent.Executors
import kotlin.random.Random


class MainActivity : AppCompatActivity() {
	private lateinit var binding: ActivityCameraBinding

	private val executor = Executors.newSingleThreadExecutor()
	private val permissions = listOf(Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE)
	private val permissionsRequestCode = Random.nextInt(0, 10000)

	private val beeper = ToneGenerator(AudioManager.STREAM_NOTIFICATION, 50)
	private var lastText = String()
	private var doSaveImage: Boolean = false

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		binding = ActivityCameraBinding.inflate(layoutInflater)
		setContentView(binding.root)

		binding.capture.setOnClickListener {
			// Disable all camera controls
			it.isEnabled = false
			doSaveImage = true
			// Re-enable camera controls
			it.isEnabled = true
		}
	}


	private fun ImageProxy.toJpeg(): ByteArray {
		//This converts the ImageProxy (from the imageAnalysis Use Case)
		//to a ByteArray (compressed as JPEG) for then to be saved for debugging purposes
		//This is the closest representation of the image that is passed to the
		//decoding algorithm.

		val yBuffer = planes[0].buffer // Y
		val vuBuffer = planes[2].buffer // VU

		val ySize = yBuffer.remaining()
		val vuSize = vuBuffer.remaining()

		val nv21 = ByteArray(ySize + vuSize)

		yBuffer.get(nv21, 0, ySize)
		vuBuffer.get(nv21, ySize, vuSize)

		val yuvImage = YuvImage(nv21, ImageFormat.NV21, this.width, this.height, null)
		val out = ByteArrayOutputStream()
		yuvImage.compressToJpeg(Rect(0, 0, yuvImage.width, yuvImage.height), 100, out)
		return out.toByteArray()
	}

	private fun saveImage(image: ImageProxy) {
		try {
			val currentMillis = System.currentTimeMillis().toString()
			val filename = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES)
				.toString() + "/" + currentMillis + "_ZXingCpp.jpg"

			File(filename).outputStream().use { out ->
				out.write(image.toJpeg())
			}
			MediaActionSound().play(MediaActionSound.SHUTTER_CLICK)
		} catch (e: Exception) {
			beeper.startTone(ToneGenerator.TONE_CDMA_SOFT_ERROR_LITE) //Fail Tone
		}
	}

	private fun bindCameraUseCases() = binding.viewFinder.post {

		val cameraProviderFuture = ProcessCameraProvider.getInstance(this)
		cameraProviderFuture.addListener({

//			val size = Size(1600, 1200)

			// Set up the view finder use case to display camera preview
			val preview = Preview.Builder()
				.setTargetAspectRatio(AspectRatio.RATIO_16_9)
//				.setTargetResolution(size)
				.build()

			// Set up the image analysis use case which will process frames in real time
			val imageAnalysis = ImageAnalysis.Builder()
				.setTargetAspectRatio(AspectRatio.RATIO_16_9) // -> 1280x720
//				.setTargetResolution(size)
				.setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
				.build()

			var frameCounter = 0
			var lastFpsTimestamp = System.currentTimeMillis()
			var runtimes: Long = 0
			var runtime2: Long = 0
			val readerJava = MultiFormatReader()
			val readerCpp = BarcodeReader()

			// Create a new camera selector each time, enforcing lens facing
			val cameraSelector = CameraSelector.Builder().requireLensFacing(CameraSelector.LENS_FACING_BACK).build()

			// Camera provider is now guaranteed to be available
			val cameraProvider = cameraProviderFuture.get()

			// Apply declared configs to CameraX using the same lifecycle owner
			cameraProvider.unbindAll()
			val camera = cameraProvider.bindToLifecycle(
				this as LifecycleOwner, cameraSelector, preview, imageAnalysis
			)

			// Reduce exposure time to decrease effect of motion blur
			val camera2 = Camera2CameraControl.from(camera.cameraControl)
			camera2.captureRequestOptions = CaptureRequestOptions.Builder()
				.setCaptureRequestOption(CaptureRequest.SENSOR_SENSITIVITY, 1600)
				.setCaptureRequestOption(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION, -8)
				.build()

			// Use the camera object to link our preview use case with the view
			preview.setSurfaceProvider(binding.viewFinder.surfaceProvider)

			imageAnalysis.setAnalyzer(executor, ImageAnalysis.Analyzer { image ->
				// Early exit: image analysis is in paused state
				if (binding.pause.isChecked) {
					image.close()
					return@Analyzer
				}

				if (doSaveImage) {
					doSaveImage = false
					saveImage(image)
				}

				val cropSize = image.height / 3 * 2
				val cropRect = if (binding.crop.isChecked)
					Rect(
						(image.width - cropSize) / 2, (image.height - cropSize) / 2,
						(image.width - cropSize) / 2 + cropSize, (image.height - cropSize) / 2 + cropSize
					)
				else
					Rect(0, 0, image.width, image.height)
				image.setCropRect(cropRect)

				val startTime = System.currentTimeMillis()
				var resultText: String
				var resultPoints: List<PointF>? = null

				if (binding.java.isChecked) {
					val yPlane = image.planes[0]
					val yBuffer = yPlane.buffer
					val yStride = yPlane.rowStride
					val data = ByteArray(yBuffer.remaining())
					yBuffer.get(data, 0, data.size)
					image.close()
					val hints = mutableMapOf<DecodeHintType, Any>()
					if (binding.qrcode.isChecked)
						hints[DecodeHintType.POSSIBLE_FORMATS] = arrayListOf(BarcodeFormat.QR_CODE)
					if (binding.tryHarder.isChecked)
						hints[DecodeHintType.TRY_HARDER] = true

					resultText = try {
						val bitmap = BinaryBitmap(
							HybridBinarizer(
								PlanarYUVLuminanceSource(
									data, yStride, image.height,
									cropRect.left, cropRect.top, cropRect.width(), cropRect.height(),
									false
								)
							)
						)
						val result = readerJava.decode(bitmap, hints)
						result?.let { "${it.barcodeFormat}: ${it.text}" } ?: ""
					} catch (e: Throwable) {
						if (e.toString() != "com.google.zxing.NotFoundException") e.toString() else ""
					}
				} else {
					readerCpp.options = BarcodeReader.Options(
						formats = if (binding.qrcode.isChecked) setOf(Format.QR_CODE) else setOf(),
						tryHarder = binding.tryHarder.isChecked,
						tryRotate = binding.tryRotate.isChecked,
						tryInvert = binding.tryInvert.isChecked,
						tryDownscale = binding.tryDownscale.isChecked
					)

					resultText = try {
						val result = readerCpp.read(image)
						runtime2 += result?.time?.toInt() ?: 0
						resultPoints = result?.position?.let {
							listOf(
								it.topLeft,
								it.topRight,
								it.bottomRight,
								it.bottomLeft
							).map { p ->
								p.toPointF()
							}
						}
						(result?.let { "${it.format} (${it.contentType}): " +
								"${if (it.contentType != BarcodeReader.ContentType.BINARY) it.text else it.bytes!!.joinToString(separator = "") { v -> "%02x".format(v) }}" }
							?: "")
					} catch (e: Throwable) {
						e.message ?: "Error"
					}
				}

				runtimes += System.currentTimeMillis() - startTime

				var infoText: String? = null
				if (++frameCounter == 15) {
					val now = System.currentTimeMillis()
					val fps = 1000 * frameCounter.toDouble() / (now - lastFpsTimestamp)

					infoText = "Time: %2d/%2d ms, FPS: %.02f, (%dx%d)"
						.format(runtimes / frameCounter, runtime2 / frameCounter, fps, image.width, image.height)
					lastFpsTimestamp = now
					frameCounter = 0
					runtimes = 0
					runtime2 = 0
				}

				camera.cameraControl.enableTorch(binding.torch.isChecked)

				showResult(resultText, infoText, resultPoints, image)
			})

		}, ContextCompat.getMainExecutor(this))
	}

	private fun showResult(resultText: String, fpsText: String?, points: List<PointF>?, image: ImageProxy) =
		binding.viewFinder.post {
			// Update the text and UI
			binding.result.text = resultText
			binding.result.visibility = View.VISIBLE

			binding.overlay.update(binding.viewFinder, image, points)

			if (fpsText != null)
				binding.fps.text = fpsText

			if (resultText.isNotEmpty() && lastText != resultText) {
				lastText = resultText
				beeper.startTone(ToneGenerator.TONE_PROP_BEEP)
			}
		}

	override fun onResume() {
		super.onResume()

		// Request permissions each time the app resumes, since they can be revoked at any time
		if (!hasPermissions(this)) {
			ActivityCompat.requestPermissions(this, permissions.toTypedArray(), permissionsRequestCode)
		} else {
			bindCameraUseCases()
		}
	}

	override fun onRequestPermissionsResult(
		requestCode: Int,
		permissions: Array<out String>,
		grantResults: IntArray,
	) {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults)
		if (requestCode == permissionsRequestCode && hasPermissions(this)) {
			bindCameraUseCases()
		} else {
			finish() // If we don't have the required permissions, we can't run
		}
	}

	private fun hasPermissions(context: Context) = permissions.all {
		ContextCompat.checkSelfPermission(context, it) == PackageManager.PERMISSION_GRANTED
	}
}