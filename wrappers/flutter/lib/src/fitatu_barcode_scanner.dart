import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

class FitatuBarcodeScanner
    implements FitatuBarcodeScannerHostApi, FitatuBarcodeScannerFlutterApi {
  FitatuBarcodeScanner({
    required this.onReady,
    required this.onSuccess,
    this.onError,
    this.onCameraImage,
    this.onTorchState,
  }) {
    FitatuBarcodeScannerFlutterApi.setup(this);
  }

  final _api = FitatuBarcodeScannerHostApi();

  final ValueChanged<int> onReady;
  final ValueChanged<String> onSuccess;
  final ValueChanged<String>? onError;
  final ValueChanged<CameraImage>? onCameraImage;
  final ValueChanged<bool>? onTorchState;

  @override
  Future<void> dispose() {
    return _api.dispose();
  }

  @override
  Future<void> init(ScannerOptions options) {
    return _api.init(options);
  }

  @override
  void ready(int textureId) => onReady(textureId);

  @override
  void result(String? code, CameraImage cameraImage, String? error) {
    if (code != null) {
      onSuccess(code);
    } else if (error != null) {
      onError?.call(error);
    }
    onCameraImage?.call(cameraImage);
  }

  @override
  Future<void> setTorchEnabled(bool isEnabled) =>
      _api.setTorchEnabled(isEnabled);

  @override
  void onTorchStateChanged(bool isEnabled) {
    onTorchState?.call(isEnabled);
  }
}
