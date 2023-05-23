package com.fitatu.barcodescanner.fitatu_barcode_scanner

import io.flutter.embedding.engine.plugins.FlutterPlugin

/** FitatuBarcodeScannerPlugin */
class FitatuBarcodeScannerPlugin : FlutterPlugin {
    private lateinit var scanner: FitatuBarcodeScanner

    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        scanner = FitatuBarcodeScanner(
            flutterPluginBinding.applicationContext,
            flutterPluginBinding.textureRegistry,
            FitatuBarcodeScannerFlutterApi(flutterPluginBinding.binaryMessenger),
        )
        FitatuBarcodeScannerHostApi.setUp(flutterPluginBinding.binaryMessenger, scanner)
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        scanner.dispose()
    }
}

