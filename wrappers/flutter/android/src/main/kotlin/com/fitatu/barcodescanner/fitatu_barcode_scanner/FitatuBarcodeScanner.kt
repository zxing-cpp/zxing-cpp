package com.fitatu.barcodescanner.fitatu_barcode_scanner

import android.content.Context
import android.graphics.Rect
import android.graphics.SurfaceTexture
import android.util.Size
import android.view.Surface
import android.view.Surface.ROTATION_0
import androidx.camera.core.Camera
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.Preview
import androidx.camera.core.SurfaceRequest
import androidx.camera.core.TorchState
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.zxingcpp.BarcodeReader
import io.flutter.view.TextureRegistry
import kotlinx.coroutines.DelicateCoroutinesApi
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.FlowPreview
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.debounce
import kotlinx.coroutines.flow.filter
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.util.concurrent.Executors


class FitatuBarcodeScanner(
    private val lifecycleOwner: LifecycleOwner,
    private val context: Context,
    private val textureRegistry: TextureRegistry,
    private val flutterApi: FitatuBarcodeScannerFlutterApi,
) :
    FitatuBarcodeScannerHostApi {

    private val barcodeReader by lazy { BarcodeReader() }
    private val targetResolution by lazy { Size(1280, 720) }

    private var camera: Camera? = null
    private var cameraProvider: ProcessCameraProvider? = null
    private var options: ScannerOptions? = null

    private lateinit var surfaceTexture: SurfaceTexture
    private var scanResultJob: Job? = null

    override fun init(options: ScannerOptions) {
        this.options = options
        barcodeReader.options = BarcodeReader.Options(
            formats = if (options.qrCode) setOf(
                BarcodeReader.Format.QR_CODE,
                BarcodeReader.Format.MICRO_QR_CODE
            ) else emptySet(),
            tryHarder = options.tryHarder,
            tryInvert = options.tryInvert,
            tryRotate = options.tryRotate,
        )

        ProcessCameraProvider.getInstance(context).apply {
            addListener(
                {
                    if (isCancelled) return@addListener
                    cameraProvider = get().apply {
                        val result = configureCamera(options, this)
                        if (result.isFailure) {
                            flutterApi.onTextureChanged(null) {}
                        }
                    }
                },
                ContextCompat.getMainExecutor(context)
            )
        }
    }

    override fun setTorchEnabled(isEnabled: Boolean) {
        camera?.cameraControl?.enableTorch(isEnabled)
    }

    @OptIn(FlowPreview::class, DelicateCoroutinesApi::class)
    private fun configureCamera(
        options: ScannerOptions,
        processCameraProvider: ProcessCameraProvider
    ) =
        processCameraProvider.runCatching {
            // Barcode reading
            val codeFlow = MutableStateFlow<ScanResult?>(null)
            scanResultJob?.cancel()
            scanResultJob = GlobalScope.launch(Dispatchers.Default) {
                codeFlow
                    .debounce {
                        if (it?.code == null) return@debounce options.scanDelay
                        return@debounce options.scanDelaySuccess
                    }
                    .filter { it != null }
                    .map { it!! }
                    .collect {
                        if (!isActive) return@collect
                        launch(Dispatchers.Main) {
                            flutterApi.result(it) {}
                        }
                    }
            }

            val imageAnalysis = ImageAnalysis.Builder()
                .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                .build().apply {
                    setAnalyzer(Executors.newSingleThreadExecutor()) { image ->
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
                            codeFlow.value = ScanResult(
                                code,
                                cameraImage,
                                null
                            )
                        } catch (e: Exception) {
                            codeFlow.value = ScanResult(
                                null,
                                cameraImage,
                                null
                            )
                        }
                        image.close()
                    }
                }

            // Surface
            val preview = Preview.Builder()
                .setTargetResolution(targetResolution)
                .setTargetRotation(ROTATION_0)
                .build()

            val cameraSelector = CameraSelector.Builder()
                .requireLensFacing(CameraSelector.LENS_FACING_BACK)
                .build()

            unbindAll()
            camera = bindToLifecycle(
                lifecycleOwner,
                cameraSelector,
                imageAnalysis,
                preview
            ).apply {
                cameraInfo.torchState.observe(lifecycleOwner) {
                    flutterApi.onTorchStateChanged(it == TorchState.ON) {}
                }
            }

            preview.setSurfaceProvider { request ->
                val (textureId, surfaceTexture) = constructSurfaceTexture()
                surfaceTexture.setDefaultBufferSize(
                    request.resolution.width,
                    request.resolution.height
                )
                this@FitatuBarcodeScanner.surfaceTexture = surfaceTexture
                val flutterSurface = Surface(this@FitatuBarcodeScanner.surfaceTexture)

                request.provideSurface(
                    flutterSurface,
                    Executors.newSingleThreadExecutor()
                ) {
                    flutterSurface.release()
                    when (it.resultCode) {
                        SurfaceRequest.Result.RESULT_REQUEST_CANCELLED,
                        SurfaceRequest.Result.RESULT_WILL_NOT_PROVIDE_SURFACE,
                        SurfaceRequest.Result.RESULT_SURFACE_ALREADY_PROVIDED,
                        SurfaceRequest.Result.RESULT_SURFACE_USED_SUCCESSFULLY -> {
                        }

                        else -> {
                            flutterApi.onTextureChanged(null) {}
                        }
                    }
                }
                flutterApi.onTextureChanged(
                    CameraConfig(
                        textureId,
                        request.resolution.width.toLong(),
                        request.resolution.height.toLong()
                    )
                ) {}
            }
        }

    /**
     * Construct [Surface] for camera preview
     *
     * @return Pair of surface id and [Surface]
     */
    private fun constructSurfaceTexture(): Pair<Long, SurfaceTexture> {
        val entry = textureRegistry.createSurfaceTexture()
        return entry.id() to entry.surfaceTexture()
    }

    override fun release() {
        scanResultJob?.cancel()
        cameraProvider?.unbindAll()
    }
}

