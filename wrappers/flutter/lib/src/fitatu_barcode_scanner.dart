import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:flutter/foundation.dart';

class FitatuBarcodeScanner extends ChangeNotifier
    implements FitatuBarcodeScannerHostApi, FitatuBarcodeScannerFlutterApi {
  FitatuBarcodeScanner({
    required this.onResult,
  }) {
    FitatuBarcodeScannerFlutterApi.setup(this);
  }

  final _api = FitatuBarcodeScannerHostApi();

  final ValueChanged<ScanResult> onResult;
  var _isTorchEnabled = false;
  bool get isTorchEnabled => _isTorchEnabled;
  CameraConfig? _cameraConfig;
  CameraConfig? get cameraConfig => _cameraConfig;
  ScanResult? _scanResult;
  ScanResult? get scanResult => _scanResult;

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
  void result(ScanResult scanResult) {
    _scanResult = scanResult;
    onResult(scanResult);
    notifyListeners();
  }

  @override
  Future<void> setTorchEnabled(bool isEnabled) =>
      _api.setTorchEnabled(isEnabled);

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
}
