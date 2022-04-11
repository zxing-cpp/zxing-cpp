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
import android.media.ToneGenerator
import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.camera2.interop.Camera2CameraControl
import androidx.camera.camera2.interop.CaptureRequestOptions
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.graphics.toPoint
import androidx.lifecycle.LifecycleOwner
import com.zxingcpp.BarcodeReader
import com.zxingcpp.BarcodeReader.Format
import com.google.zxing.*
import com.google.zxing.common.HybridBinarizer
import java.util.concurrent.Executors
import kotlin.math.abs
import kotlin.random.Random
import com.example.zxingcppdemo.databinding.ActivityCameraBinding

class MainActivity : AppCompatActivity() {
	private lateinit var binding: ActivityCameraBinding
	private val executor = Executors.newSingleThreadExecutor()
	private val permissions = listOf(Manifest.permission.CAMERA)
	private val permissionsRequestCode = Random.nextInt(0, 10000)

	private val beeper = ToneGenerator(AudioManager.STREAM_NOTIFICATION, 50)
	private var lastText = String()
	private var cropRect = Rect()
	private var imageRotation: Int = 0
	private var imageWidth: Int = 0
	private var imageHeight: Int = 0

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		binding = ActivityCameraBinding.inflate(layoutInflater)
		setContentView(binding.root)

		binding.capture.setOnClickListener {
			// Disable all camera controls
			it.isEnabled = false

			//TODO: save image

			// Re-enable camera controls
			it.isEnabled = true
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

			imageAnalysis.setAnalyzer(executor, ImageAnalysis.Analyzer { image ->
				imageRotation = image.imageInfo.rotationDegrees
				imageWidth = image.width
				imageHeight = image.height

				// Early exit: image analysis is in paused state
				if (binding.pause.isChecked) {
					image.close()
					return@Analyzer
				}

				val cropSize = image.height / 3 * 2
				cropRect = if (binding.crop.isChecked)
					Rect(
						(image.width - cropSize) / 2, (image.height - cropSize) / 2,
						(image.width - cropSize) / 2 + cropSize, (image.height - cropSize) / 2 + cropSize
					)
				else
					Rect(0, 0, image.width, image.height)

				val startTime = System.currentTimeMillis()
				var resultText: String

				if (binding.java.isChecked) {
					val yBuffer = image.planes[0].buffer // Y
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
									data, imageWidth, imageHeight,
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
						tryRotate = binding.tryRotate.isChecked
					)

					image.setCropRect(cropRect)

					resultText = try {
						val result = readerCpp.read(image)
						runtime2 += result?.time?.toInt() ?: 0
						(result?.let { "${it.format}: ${it.text}" } ?: "")
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

				showResult(resultText, infoText)
			})

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

		}, ContextCompat.getMainExecutor(this))
	}

	private fun showResult(resultText: String, fpsText: String?) = binding.viewFinder.post {
		// Update the text and UI
		binding.result.text = resultText
		binding.result.visibility = View.VISIBLE

		val tl = image2View(Point(cropRect.left, cropRect.top))
		val br = image2View(Point(cropRect.right, cropRect.bottom))
		(binding.cropRect.layoutParams as ViewGroup.MarginLayoutParams).apply {
			leftMargin = kotlin.math.min(tl.x, br.x)
			topMargin = kotlin.math.min(tl.y, br.y)
			width = abs(br.x - tl.x)
			height = abs(br.y - tl.y)
		}
		binding.cropRect.visibility = if (binding.crop.isChecked) View.VISIBLE else View.GONE

		if (fpsText != null)
			binding.fps.text = fpsText

		if (resultText.isNotEmpty() && lastText != resultText) {
			lastText = resultText
			beeper.startTone(ToneGenerator.TONE_PROP_BEEP)
		}
	}

	private fun image2View(p: Point) : Point {
		val s = kotlin.math.min(binding.viewFinder.width, binding.viewFinder.height).toFloat() / imageHeight
		val o = (kotlin.math.max(binding.viewFinder.width, binding.viewFinder.height) - (imageWidth * s).toInt()) / 2
		val res = PointF(p.x * s + o, p.y * s).toPoint()
		return if (imageRotation % 180 == 0) res else Point(binding.viewFinder.width - res.y, res.x)
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
