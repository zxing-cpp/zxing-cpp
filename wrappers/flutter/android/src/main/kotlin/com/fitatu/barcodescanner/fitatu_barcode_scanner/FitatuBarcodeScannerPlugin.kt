package com.fitatu.barcodescanner.fitatu_barcode_scanner

import androidx.core.app.ComponentActivity
import androidx.lifecycle.LifecycleOwner
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.embedding.engine.plugins.activity.ActivityAware
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding
import kotlinx.coroutines.CoroutineScope
import kotlin.concurrent.fixedRateTimer

/** FitatuBarcodeScannerPlugin */
class FitatuBarcodeScannerPlugin : FlutterPlugin, ActivityAware {
    private lateinit var scanner: FitatuBarcodeScanner
    private lateinit var flutterPluginBinding: FlutterPlugin.FlutterPluginBinding

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        this.flutterPluginBinding = flutterPluginBinding
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
    }

    override fun onAttachedToActivity(binding: ActivityPluginBinding) {
        scanner = FitatuBarcodeScanner(
            binding.activity as LifecycleOwner,
            flutterPluginBinding.applicationContext,
            flutterPluginBinding.textureRegistry,
            FitatuBarcodeScannerFlutterApi(flutterPluginBinding.binaryMessenger),
        )
        FitatuBarcodeScannerHostApi.setUp(flutterPluginBinding.binaryMessenger, scanner)
    }

    override fun onDetachedFromActivityForConfigChanges() {
    }

    override fun onReattachedToActivityForConfigChanges(binding: ActivityPluginBinding) {
    }

    override fun onDetachedFromActivity() {
        scanner.release()
    }
}

