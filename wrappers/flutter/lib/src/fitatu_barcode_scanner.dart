import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:flutter/foundation.dart';

class FitatuBarcodeScanner extends ChangeNotifier
    implements FitatuBarcodeScannerHostApi, FitatuBarcodeScannerFlutterApi {
  FitatuBarcodeScanner({
    required this.onSuccess,
  }) {
    FitatuBarcodeScannerFlutterApi.setup(this);
  }

  final _api = FitatuBarcodeScannerHostApi();

  final ValueChanged<String> onSuccess;
  var _isTorchEnabled = false;
  bool get isTorchEnabled => _isTorchEnabled;
  int? _textureId;
  int? get textureId => _textureId;
  CameraImage? _cameraImage;
  CameraImage? get cameraImage => _cameraImage;

  @override
  void dispose() async {
    await release();
    super.dispose();
  }

  @override
  Future<void> release() async {
    _textureId = null;
    notifyListeners();
    await _api.release();
  }

  @override
  Future<void> init(ScannerOptions options) {
    return _api.init(options);
  }

  @override
  void ready(int textureId) {
    _textureId = textureId;
    notifyListeners();
  }

  @override
  void result(String? code, CameraImage cameraImage, String? error) {
    _cameraImage = cameraImage;
    notifyListeners();
    if (code != null) {
      onSuccess(code);
    }
  }

  @override
  Future<void> setTorchEnabled(bool isEnabled) =>
      _api.setTorchEnabled(isEnabled);

  @override
  Future<void> onMovedToBackground() => _api.onMovedToBackground();

  @override
  Future<void> onMovedToForeground() => _api.onMovedToForeground();

  @override
  void onTorchStateChanged(bool isEnabled) {
    _isTorchEnabled = isEnabled;
    notifyListeners();
  }
}
