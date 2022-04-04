package com.example.zxingcppdemo

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.Point
import android.graphics.Rect
import android.media.AudioManager
import android.media.ToneGenerator
import android.os.Bundle
import android.view.View
import android.view.ViewGroup
import android.widget.CheckBox
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.AspectRatio
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.google.zxing.*
import com.google.zxing.common.HybridBinarizer
import com.nubook.android.zxingcpp.ZxingCpp
import com.nubook.android.zxingcpp.ZxingCpp.Format
import java.util.concurrent.Executors
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min
import kotlin.math.roundToInt

class MainActivity : AppCompatActivity() {
	private val executor = Executors.newSingleThreadExecutor()
	private val permissions = listOf(Manifest.permission.CAMERA)
	private val beeper = ToneGenerator(AudioManager.STREAM_NOTIFICATION, 50)
	private val cropRect = Rect()

	private lateinit var viewFinder: PreviewView
	private lateinit var resultView: TextView
	private lateinit var fpsView: TextView
	private lateinit var boxCrop: View
	private lateinit var chipJava: CheckBox
	private lateinit var chipQrCode: CheckBox
	private lateinit var chipTryHarder: CheckBox
	private lateinit var chipTryRotate: CheckBox
	private lateinit var chipCrop: CheckBox
	private lateinit var chipPause: CheckBox

	private var lastText = ""
	private var imageRotation = 0
	private var imageWidth = 0
	private var imageHeight = 0

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_main)

		viewFinder = findViewById(R.id.view_finder)
		resultView = findViewById(R.id.text_result)
		fpsView = findViewById(R.id.text_fps)
		boxCrop = findViewById(R.id.box_crop)
		chipJava = findViewById(R.id.chip_java)
		chipQrCode = findViewById(R.id.chip_qrcode)
		chipTryHarder = findViewById(R.id.chip_tryHarder)
		chipTryRotate = findViewById(R.id.chip_tryRotate)
		chipCrop = findViewById(R.id.chip_crop)
		chipPause = findViewById(R.id.chip_pause)
	}

	private fun bindCameraUseCases() = viewFinder.post {
		val cameraProviderFuture = ProcessCameraProvider.getInstance(this)
		cameraProviderFuture.addListener({
			// Set up the view finder use case to display camera preview
			val preview = Preview.Builder()
				.setTargetAspectRatio(AspectRatio.RATIO_16_9)
				.build()

			// Set up the image analysis use case which will process frames in real time
			val imageAnalysis = ImageAnalysis.Builder()
				.setTargetAspectRatio(AspectRatio.RATIO_16_9) // -> 1280x720
				.setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
				.build()

			val readerJava = MultiFormatReader()

			var frameCounter = 0
			var lastFpsTimestamp = System.currentTimeMillis()
			var runtimes = 0L

			imageAnalysis.setAnalyzer(executor, ImageAnalysis.Analyzer { image ->
				imageRotation = image.imageInfo.rotationDegrees
				imageWidth = image.width
				imageHeight = image.height

				// Early exit: image analysis is in paused state
				if (chipPause.isChecked) {
					image.close()
					return@Analyzer
				}

				val cropSize = image.height / 3 * 2
				if (chipCrop.isChecked) {
					cropRect.set(
						(image.width - cropSize) / 2,
						(image.height - cropSize) / 2,
						(image.width - cropSize) / 2 + cropSize,
						(image.height - cropSize) / 2 + cropSize
					)
				} else {
					cropRect.set(0, 0, image.width, image.height)
				}

				val startTime = System.currentTimeMillis()
				val resultText = if (chipJava.isChecked) {
					val yBuffer = image.planes[0].buffer // Y
					val data = ByteArray(yBuffer.remaining())
					yBuffer.get(data, 0, data.size)
					image.close()
					val hints = mutableMapOf<DecodeHintType, Any>()
					if (chipQrCode.isChecked) {
						hints[DecodeHintType.POSSIBLE_FORMATS] = arrayListOf(BarcodeFormat.QR_CODE)
					}
					if (chipTryHarder.isChecked) {
						hints[DecodeHintType.TRY_HARDER] = true
					}
					try {
						val bitmap = BinaryBitmap(
							HybridBinarizer(
								PlanarYUVLuminanceSource(
									data, imageWidth, imageHeight,
									cropRect.left, cropRect.top,
									cropRect.width(), cropRect.height(),
									false
								)
							)
						)
						readerJava.decode(bitmap, hints)?.let {
							"${it.barcodeFormat}: ${it.text}"
						} ?: ""
					} catch (e: Throwable) {
						val s = e.toString()
						if (s != "com.google.zxing.NotFoundException") {
							s
						} else {
							""
						}
					}
				} else {
					image.setCropRect(cropRect)
					try {
						val yPlane = image.planes[0]
						val result = ZxingCpp.readYBuffer(
							yPlane.buffer,
							yPlane.rowStride,
							cropRect,
							image.imageInfo.rotationDegrees,
							if (chipQrCode.isChecked) {
								setOf(Format.QR_CODE)
							} else {
								setOf()
							},
							chipTryHarder.isChecked,
							chipTryRotate.isChecked
						)
						result?.let {
							"${it.format}: ${it.text}"
						} ?: ""
					} catch (e: Throwable) {
						e.message ?: "Error"
					} finally {
						image.close()
					}
				}

				runtimes += System.currentTimeMillis() - startTime

				var infoText: String? = null
				if (++frameCounter == 15) {
					val now = System.currentTimeMillis()
					val fps = 1000 * frameCounter.toDouble() / (now - lastFpsTimestamp)
					infoText = "Time: %2dms, FPS: %.02f, (%dx%d)"
						.format(
							runtimes / frameCounter,
							fps,
							image.width,
							image.height
						)
					lastFpsTimestamp = now
					frameCounter = 0
					runtimes = 0
				}

				showResult(resultText, infoText)
			})

			// Create a new camera selector each time, enforcing lens facing
			val cameraSelector = CameraSelector.Builder().requireLensFacing(
				CameraSelector.LENS_FACING_BACK
			).build()

			// Camera provider is now guaranteed to be available
			val cameraProvider = cameraProviderFuture.get()

			// Apply declared configs to CameraX using the same lifecycle owner
			cameraProvider.unbindAll()
			cameraProvider.bindToLifecycle(
				this as LifecycleOwner, cameraSelector, preview, imageAnalysis
			)

			// Use the camera object to link our preview use case with the view
			preview.setSurfaceProvider(viewFinder.surfaceProvider)
		}, ContextCompat.getMainExecutor(this))
	}

	private fun showResult(
		resultText: String,
		fpsText: String?
	) = viewFinder.post {
		// Update the text and UI
		resultView.text = resultText
		resultView.visibility = View.VISIBLE

		val tl = image2View(cropRect.left, cropRect.top)
		val br = image2View(cropRect.right, cropRect.bottom)
		(boxCrop.layoutParams as ViewGroup.MarginLayoutParams).apply {
			leftMargin = min(tl.x, br.x)
			topMargin = min(tl.y, br.y)
			width = abs(br.x - tl.x)
			height = abs(br.y - tl.y)
		}
		boxCrop.visibility = if (chipCrop.isChecked) View.VISIBLE else View.GONE

		if (fpsText != null) {
			fpsView.text = fpsText
		}

		if (resultText.isNotEmpty() && lastText != resultText) {
			lastText = resultText
			beeper.startTone(ToneGenerator.TONE_PROP_BEEP)
		}
	}

	private fun image2View(x: Int, y: Int): Point {
		val s = min(viewFinder.width, viewFinder.height).toFloat() / imageHeight
		val o = (max(viewFinder.width, viewFinder.height) - (imageWidth * s).roundToInt()) / 2
		val res = Point(
			(x * s + o).roundToInt(),
			(y * s).roundToInt()
		)
		return if (imageRotation % 180 == 0) {
			res
		} else {
			Point(viewFinder.width - res.y, res.x)
		}
	}

	override fun onResume() {
		super.onResume()

		// Request permissions each time the app resumes, since they can be revoked at any time
		if (!hasPermissions(this)) {
			ActivityCompat.requestPermissions(
				this,
				permissions.toTypedArray(),
				PERMISSION_REQUEST_CODE
			)
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
		if (requestCode == PERMISSION_REQUEST_CODE && hasPermissions(this)) {
			bindCameraUseCases()
		} else {
			finish() // If we don't have the required permissions, we can't run
		}
	}

	private fun hasPermissions(context: Context) = permissions.all {
		ContextCompat.checkSelfPermission(context, it) == PackageManager.PERMISSION_GRANTED
	}

	companion object {
		private const val PERMISSION_REQUEST_CODE = 1
	}
}
