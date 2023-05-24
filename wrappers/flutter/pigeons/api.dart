import 'package:pigeon/pigeon.dart';

@HostApi()
abstract class FitatuBarcodeScannerHostApi {
  void init(ScannerOptions options);
  void setTorchEnabled(bool isEnabled);
  void release();
}

@FlutterApi()
abstract class FitatuBarcodeScannerFlutterApi {
  void onTextureChanged(CameraConfig? cameraConfig);
  void result(String? code, CameraImage cameraImage, String? error);
  void onTorchStateChanged(bool isEnabled);
}

class CameraConfig {
  final int textureId;
  final int previewWidth;
  final int previewHeight;

  CameraConfig(this.textureId, this.previewWidth, this.previewHeight);
}

class CameraImage {
  final CropRect cropRect;
  final int width;
  final int height;
  final int rotationDegrees;

  CameraImage({
    required this.cropRect,
    required this.width,
    required this.height,
    required this.rotationDegrees,
  });
}

class CropRect {
  final int left;
  final int top;
  final int right;
  final int bottom;

  CropRect({
    required this.left,
    required this.top,
    required this.right,
    required this.bottom,
  });
}

class ScannerOptions {
  final bool tryHarder;
  final bool tryRotate;
  final bool tryInvert;
  final bool qrCode;
  final double cropPercent;

  const ScannerOptions({
    required this.tryHarder,
    required this.tryRotate,
    required this.tryInvert,
    required this.qrCode,
    required this.cropPercent,
  });
}
