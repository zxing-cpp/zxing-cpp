/*
* Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

package zxingcpp.app

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.ImageFormat
import android.graphics.PointF
import android.graphics.Rect
import android.graphics.YuvImage
import android.hardware.camera2.CaptureRequest
import android.media.AudioManager
import android.media.MediaActionSound
import android.media.ToneGenerator
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.view.View
import androidx.annotation.OptIn
import androidx.appcompat.app.AppCompatActivity
import android.util.Size
import androidx.camera.camera2.interop.Camera2CameraControl
import androidx.camera.camera2.interop.CaptureRequestOptions
import androidx.camera.camera2.interop.ExperimentalCamera2Interop
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageProxy
import androidx.camera.view.CameraController
import androidx.camera.view.LifecycleCameraController
import androidx.camera.core.resolutionselector.AspectRatioStrategy
import androidx.camera.core.resolutionselector.ResolutionSelector
import androidx.camera.core.resolutionselector.ResolutionStrategy
import androidx.core.content.ContextCompat
import androidx.core.graphics.toPointF
import androidx.activity.result.contract.ActivityResultContracts
import com.google.zxing.BarcodeFormat
import com.google.zxing.BinaryBitmap
import com.google.zxing.DecodeHintType
import com.google.zxing.MultiFormatReader
import com.google.zxing.PlanarYUVLuminanceSource
import com.google.zxing.common.HybridBinarizer
import java.io.ByteArrayOutputStream
import java.io.File
import java.util.concurrent.Executors
import zxingcpp.app.databinding.ActivityCameraBinding
import zxingcpp.BarcodeReader
import zxingcpp.BarcodeReader.Format.*

class MainActivity : AppCompatActivity() {
	private lateinit var binding: ActivityCameraBinding
	private lateinit var cameraController: LifecycleCameraController

	private val executor = Executors.newSingleThreadExecutor()
	private val permissions = mutableListOf(Manifest.permission.CAMERA)
	private val beeper = ToneGenerator(AudioManager.STREAM_NOTIFICATION, 50)

	private val requestPermissionsLauncher =
		registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { permissions ->
			if (permissions.all { it.value }) {
				bindCameraUseCases()
			} else {
				finish()
			}
		}

	private var lastText = String()
	private var doSaveImage: Boolean = false

	init {
		// On R or higher, this permission has no effect. See:
		// https://developer.android.com/reference/android/Manifest.permission#WRITE_EXTERNAL_STORAGE
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
			permissions.add(Manifest.permission.WRITE_EXTERNAL_STORAGE)
		}
	}

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

		binding.torch.setOnCheckedChangeListener { _, isChecked ->
			if (::cameraController.isInitialized) {
				cameraController.cameraControl?.enableTorch(isChecked)
			}
		}
	}

	private fun ImageProxy.toJpeg(): ByteArray {
		// This converts the ImageProxy (from the imageAnalysis Use Case)
		// to a ByteArray (compressed as JPEG) for then to be saved for debugging purposes
		// This is the closest representation of the image that is passed to the
		// decoding algorithm.

		val yBuffer = planes[0].buffer // Y
		val vuBuffer = planes[2].buffer // VU

		val ySize = yBuffer.remaining()
		val vuSize = vuBuffer.remaining()

		val nv21 = ByteArray(ySize + vuSize)

		yBuffer.get(nv21, 0, ySize)
		vuBuffer.get(nv21, ySize, vuSize)

		val yuvImage = YuvImage(nv21, ImageFormat.NV21, this.width, this.height, null)
		val out = ByteArrayOutputStream()
		yuvImage.compressToJpeg(Rect(0, 0, yuvImage.width, yuvImage.height), 90, out)
		return out.toByteArray()
	}

	private fun saveImage(image: ImageProxy) {
		try {
			val currentMillis = System.currentTimeMillis().toString()
			val filename =
				Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES)
					.toString() + "/" + currentMillis + "_ZXingCpp.jpg"

			File(filename).outputStream().use { out ->
				out.write(image.toJpeg())
			}
			MediaActionSound().play(MediaActionSound.SHUTTER_CLICK)
		} catch (e: Exception) {
			beeper.startTone(ToneGenerator.TONE_CDMA_SOFT_ERROR_LITE) //Fail Tone
		}
	}

	@OptIn(ExperimentalCamera2Interop::class)
	private fun bindCameraUseCases() = binding.viewFinder.post {
		if (::cameraController.isInitialized) {
			return@post
		}

		cameraController = LifecycleCameraController(this)
		cameraController.bindToLifecycle(this)
		cameraController.cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA
		cameraController.setEnabledUseCases(CameraController.IMAGE_ANALYSIS)

		val resolutionSelector = ResolutionSelector.Builder()
			.setAspectRatioStrategy(AspectRatioStrategy.RATIO_16_9_FALLBACK_AUTO_STRATEGY)
			.build()
		cameraController.setPreviewResolutionSelector(resolutionSelector)

		val analysisResolutionSelector = ResolutionSelector.Builder()
			.setAspectRatioStrategy(AspectRatioStrategy.RATIO_16_9_FALLBACK_AUTO_STRATEGY)
			.setResolutionStrategy(
				ResolutionStrategy(
					Size(1280, 720),
					ResolutionStrategy.FALLBACK_RULE_CLOSEST_HIGHER_THEN_LOWER
				)
			)
			.build()
		cameraController.setImageAnalysisResolutionSelector(analysisResolutionSelector)

		var frameCounter = 0
		var lastFpsTimestamp = System.currentTimeMillis()
		var runtimes: Long = 0
		var runtime2: Long = 0
		val readerJava = MultiFormatReader()
		val readerCpp = BarcodeReader()

		cameraController.setImageAnalysisAnalyzer(executor) { image ->
			// Early exit: image analysis is in paused state
			if (binding.pause.isChecked) {
				image.close()
				return@setImageAnalysisAnalyzer
			}

			if (doSaveImage) {
				doSaveImage = false
				saveImage(image)
			}

			val cropSize = image.height / 3 * 2
			val cropRect = if (binding.crop.isChecked)
				Rect(
					(image.width - cropSize) / 2,
					(image.height - cropSize) / 2,
					(image.width - cropSize) / 2 + cropSize,
					(image.height - cropSize) / 2 + cropSize
				)
			else
				Rect(0, 0, image.width, image.height)
			image.setCropRect(cropRect)

			val startTime = System.currentTimeMillis()
			var resultText: String
			val resultPoints = mutableListOf<List<PointF>>()

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
				if (binding.tryInvert.isChecked)
					hints[DecodeHintType.ALSO_INVERTED] = true

				resultText = try {
					val bitmap = BinaryBitmap(
						HybridBinarizer(
							PlanarYUVLuminanceSource(
								data,
								yStride,
								image.height,
								cropRect.left,
								cropRect.top,
								cropRect.width(),
								cropRect.height(),
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
				readerCpp.options.apply {
					formats = if (binding.qrcode.isChecked) setOf(QR_CODE, MICRO_QR_CODE, RMQR_CODE) else setOf()
					tryHarder = binding.tryHarder.isChecked
					tryRotate = binding.tryRotate.isChecked
					tryInvert = binding.tryInvert.isChecked
					tryDownscale = binding.tryDownscale.isChecked
					maxNumberOfSymbols = if (binding.multiSymbol.isChecked) 255 else 1
				}

				resultText = try {
					image.use {
						readerCpp.read(it)
					}.joinToString("\n") { result ->
						result.position.let {
							resultPoints.add(listOf(
								it.topLeft, it.topRight, it.bottomRight, it.bottomLeft
							).map { p ->
								p.toPointF()
							})
						}
						"${result.format} (${result.contentType}): ${
							if (result.contentType != BarcodeReader.ContentType.BINARY) {
								result.text
							} else {
								result.bytes!!.joinToString(separator = "") { v -> "%02x".format(v) }
							}
						}"
					}
				} catch (e: Throwable) {
					e.message ?: "Error"
				}
			}

			runtimes += System.currentTimeMillis() - startTime
			runtime2 += readerCpp.lastReadTime

			var infoText: String? = null
			if (++frameCounter == 15) {
				val now = System.currentTimeMillis()
				val fps = 1000 * frameCounter.toDouble() / (now - lastFpsTimestamp)

				infoText = "Time: %2d/%2d ms, FPS: %.02f, (%dx%d)".format(
					runtimes / frameCounter, runtime2 / frameCounter, fps, image.width, image.height
				)
				lastFpsTimestamp = now
				frameCounter = 0
				runtimes = 0
				runtime2 = 0
			}

			showResult(resultText, infoText, resultPoints, image)
		}

		// Reduce exposure time to decrease effect of motion blur
		cameraController.initializationFuture.addListener({
			cameraController.cameraControl?.let {
				val camera2 = Camera2CameraControl.from(it)
				camera2.captureRequestOptions = CaptureRequestOptions.Builder()
					.setCaptureRequestOption(CaptureRequest.SENSOR_SENSITIVITY, 1600)
					.setCaptureRequestOption(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION, -8)
					.build()
			}
		}, ContextCompat.getMainExecutor(this))

		binding.viewFinder.controller = cameraController
	}

	private fun showResult(
		resultText: String,
		fpsText: String?,
		points: List<List<PointF>>,
		image: ImageProxy
	) =
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
			requestPermissionsLauncher.launch(permissions.toTypedArray())
		} else {
			bindCameraUseCases()
		}
	}

	private fun hasPermissions(context: Context) = permissions.all {
		ContextCompat.checkSelfPermission(context, it) == PackageManager.PERMISSION_GRANTED
	}
}
