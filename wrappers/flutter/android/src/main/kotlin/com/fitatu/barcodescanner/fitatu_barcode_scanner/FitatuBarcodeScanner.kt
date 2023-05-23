package com.fitatu.barcodescanner.fitatu_barcode_scanner

import android.content.Context
import android.graphics.Rect
import android.util.Size
import android.view.Surface
import androidx.camera.core.Camera
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.Preview
import androidx.camera.core.TorchState
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleOwner
import androidx.lifecycle.LifecycleRegistry
import com.google.common.util.concurrent.ListenableFuture
import com.zxingcpp.BarcodeReader
import io.flutter.view.TextureRegistry
import java.util.concurrent.Executors

class FitatuBarcodeScanner(
    private val context: Context,
    private val textureRegistry: TextureRegistry,
    private val flutterApi: FitatuBarcodeScannerFlutterApi,
) :
    FitatuBarcodeScannerHostApi {

    private var cameraProviderFuture: ListenableFuture<ProcessCameraProvider>? = null
    private var cameraProvider: ProcessCameraProvider? = null
    private val imageAnalysisExecutor by lazy { Executors.newFixedThreadPool(2) }
    private val mainThreadExecutor by lazy { ContextCompat.getMainExecutor(context) }
    private val barcodeReader by lazy { BarcodeReader() }
    private val targetResolution by lazy { Size(1280, 720) }

    private val preview by lazy {
        Preview.Builder()
            .setTargetResolution(targetResolution)
            .build()
    }
    private val cameraSelector by lazy {
        CameraSelector.Builder()
            .requireLensFacing(CameraSelector.LENS_FACING_BACK)
            .build()
    }
    private val imageAnalysis by lazy {
        ImageAnalysis.Builder()
            .setTargetResolution(targetResolution)
            .build()
    }

    private val flutterLifecycleOwner by lazy { FlutterLifecycleOwner() }

    private var camera: Camera? = null

    override fun init(options: ScannerOptions) {
        flutterLifecycleOwner.onCreate()
        cameraProviderFuture = ProcessCameraProvider.getInstance(context).apply {
            addListener(
                {
                    if (isCancelled) return@addListener
                    cameraProvider = get().apply { configureCamera(options, this) }
                },
                mainThreadExecutor
            )
        }
    }

    override fun setTorchEnabled(isEnabled: Boolean) {
        camera?.cameraControl?.enableTorch(isEnabled)
    }

    override fun onMovedToForeground() {
        flutterLifecycleOwner.onStart()
        flutterLifecycleOwner.onResume()
    }

    override fun onMovedToBackground() {
        flutterLifecycleOwner.onPause()
        flutterLifecycleOwner.onStop()
    }

    private fun configureCamera(
        options: ScannerOptions,
        processCameraProvider: ProcessCameraProvider
    ) =
        processCameraProvider.apply {
            // Surface
            val (textureId, surface) = constructSurface()
            preview.setSurfaceProvider { request ->
                request.provideSurface(
                    surface,
                    mainThreadExecutor
                ) {

                }
            }
            flutterApi.ready(textureId) {}

            // Barcode reading
            barcodeReader.options = BarcodeReader.Options(
                formats = if (options.qrCode) setOf(
                    BarcodeReader.Format.QR_CODE,
                    BarcodeReader.Format.MICRO_QR_CODE
                ) else emptySet(),
                tryHarder = options.tryHarder,
                tryInvert = options.tryInvert,
                tryRotate = options.tryRotate,
            )

            imageAnalysis.setAnalyzer(imageAnalysisExecutor) { image ->
                val cropSize = image.height.times(options.cropPercent).toInt()

                val cropRect = Rect(
                    (image.width - cropSize) / 2,
                    (image.height - cropSize) / 2,
                    (image.width - cropSize) / 2 + cropSize,
                    (image.height - cropSize) / 2 + cropSize
                )
                image.setCropRect(cropRect)

                val cameraImage = CameraImage(
                    cropRect = CropRect(
                        left = cropRect.left.toLong(),
                        top = cropRect.top.toLong(),
                        right = cropRect.right.toLong(),
                        bottom = cropRect.bottom.toLong()
                    ),
                    width = image.width.toLong(),
                    height = image.height.toLong(),
                    rotationDegrees = image.imageInfo.rotationDegrees.toLong()
                )

                try {
                    val result = barcodeReader.read(image)
                    val code = result?.text?.trim()?.takeIf { it.isNotBlank() }
                    mainThreadExecutor.execute {
                        flutterApi.result(code, cameraImage, null) {}
                    }
                } catch (e: Exception) {
                    flutterApi.result(null, cameraImage, e.message) {}
                } finally {
                    image.close()
                }
            }

            camera = bindToLifecycle(
                flutterLifecycleOwner,
                cameraSelector,
                imageAnalysis,
                preview
            ).apply {
                cameraInfo.torchState.observe(flutterLifecycleOwner) {
                    flutterApi.onTorchStateChanged(it == TorchState.ON) {}
                }
            }

            flutterLifecycleOwner.onStart()
            flutterLifecycleOwner.onResume()
        }

    /**
     * Construct [Surface] for camera preview
     *
     * @return Pair of surface id and [Surface]
     */
    private fun constructSurface(): Pair<Long, Surface> {
        val entry = textureRegistry.createSurfaceTexture()
        return entry.id() to entry.surfaceTexture()
            .apply {
                setDefaultBufferSize(targetResolution.width, targetResolution.height)
            }
            .let(::Surface)
    }

    override fun release() {
        flutterLifecycleOwner.onPause()
        flutterLifecycleOwner.onStop()
        flutterLifecycleOwner.onDestroy()
    }
}

private class FlutterLifecycleOwner : LifecycleOwner {

    private val lifecycleRegistry = LifecycleRegistry(this).apply {
        currentState = Lifecycle.State.CREATED
    }

    fun onCreate() {
        lifecycleRegistry.handleLifecycleEvent(Lifecycle.Event.ON_CREATE)
    }

    fun onStart() {
        lifecycleRegistry.handleLifecycleEvent(Lifecycle.Event.ON_START)
    }

    fun onResume() {
        lifecycleRegistry.handleLifecycleEvent(Lifecycle.Event.ON_RESUME)
    }

    fun onPause() {
        lifecycleRegistry.handleLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    }

    fun onStop() {
        lifecycleRegistry.handleLifecycleEvent(Lifecycle.Event.ON_STOP)
    }

    fun onDestroy() {
        lifecycleRegistry.handleLifecycleEvent(Lifecycle.Event.ON_DESTROY)
    }

    override fun getLifecycle(): Lifecycle = lifecycleRegistry
}



