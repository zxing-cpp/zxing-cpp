import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:flutter/foundation.dart';

typedef ScannerErrorCallback = void Function(String? error);

class FitatuBarcodeScanner extends ChangeNotifier implements FitatuBarcodeScannerHostApi, FitatuBarcodeScannerFlutterApi {
  FitatuBarcodeScanner({
    required this.onResult,
    required this.onError,
  }) {
    FitatuBarcodeScannerFlutterApi.setup(this);
  }

  final _api = FitatuBarcodeScannerHostApi();

  final ValueChanged<String?> onResult;
  final ScannerErrorCallback? onError;
  var _isTorchEnabled = false;
  bool get isTorchEnabled => _isTorchEnabled;
  CameraConfig? _cameraConfig;
  CameraConfig? get cameraConfig => _cameraConfig;
  String? _code;
  String? get code => _code;
  String? _error;
  String? get error => _error;
  CameraImage? _cameraImage;
  CameraImage? get cameraImage => _cameraImage;

  var _isDisposed = false;

  @override
  void dispose() async {
    _isDisposed = true;
    await release();
    super.dispose();
  }

  @override
  Future<void> release() async {
    _cameraConfig = null;
    notifyListeners();
    await _api.release();
  }

  @override
  Future<void> init(ScannerOptions options) {
    return _api.init(options);
  }

  @override
  void onTextureChanged(CameraConfig? cameraConfig) {
    _cameraConfig = cameraConfig;
    notifyListeners();
  }

  @override
  void result(String? code) {
    _code = code;
    _error = null;
    onResult(code);
    notifyListeners();
  }

  @override
  Future<void> setTorchEnabled(bool isEnabled) => _api.setTorchEnabled(isEnabled);

  @override
  void onTorchStateChanged(bool isEnabled) {
    _isTorchEnabled = isEnabled;
    notifyListeners();
  }

  @override
  void notifyListeners() {
    if (_isDisposed) return;
    super.notifyListeners();
  }

  @override
  void onScanError(String error) {
    _error = error;
    onError?.call(error);
    notifyListeners();
  }

  @override
  void onCameraImage(CameraImage cameraImage) {
    _cameraImage = cameraImage;
    notifyListeners();
  }
}
