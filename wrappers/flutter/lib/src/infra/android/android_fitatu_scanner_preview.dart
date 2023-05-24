import 'dart:math';

import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:flutter/material.dart';

import '../../fitatu_barcode_scanner.dart';
import '../../scanner_preview_mixin.dart';

class AndroidFitatuScannerPreview extends StatefulWidget {
  const AndroidFitatuScannerPreview({
    super.key,
    required this.onSuccess,
    required this.options,
    this.onChanged,
  });

  final ScannerOptions options;
  final ValueChanged<String> onSuccess;
  final VoidCallback? onChanged;

  @override
  State<AndroidFitatuScannerPreview> createState() =>
      AndroidFitatuScannerPreviewState();
}

class AndroidFitatuScannerPreviewState
    extends State<AndroidFitatuScannerPreview> with ScannerPreviewMixin {
  late FitatuBarcodeScanner _scanner;

  void setStateIfMounted() {
    if (mounted) {
      setState(() {});
    }
  }

  @override
  void initState() {
    super.initState();
    _scanner = FitatuBarcodeScanner(onSuccess: widget.onSuccess)
      ..addListener(_scannerListener);
    _scanner.init(widget.options);
  }

  void _scannerListener() {
    setStateIfMounted();
    widget.onChanged?.call();
  }

  @override
  void dispose() {
    _scanner
      ..removeListener(_scannerListener)
      ..release();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final cameraConfig = _scanner.cameraConfig;
    if (cameraConfig != null) {
      final cameraAspectRatio =
          cameraConfig.previewHeight / cameraConfig.previewWidth;

      return AspectRatio(
        aspectRatio: cameraConfig.previewWidth / cameraConfig.previewHeight,
        child: Texture(
          key: ValueKey(cameraConfig),
          textureId: cameraConfig.textureId,
        ),
      );
    }
    return const SizedBox.shrink();
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    _scanner.setTorchEnabled(isEnabled);
  }

  @override
  bool isTorchEnabled() => _scanner.isTorchEnabled;
}

/// Painter which takes [cameraImage] and
/// paints bordered rectangle with same bounds
/// as android [ImageProxy.cropRect](https://developer.android.com/reference/androidx/camera/core/ImageProxy#getCropRect())
class CropRectPainter extends CustomPainter {
  final CameraImage cameraImage;
  final Paint rectPaint;

  const CropRectPainter(this.cameraImage, this.rectPaint);

  @override
  void paint(Canvas canvas, Size size) {
    final cropRect = Rect.fromLTRB(
      cameraImage.cropRect.left.toDouble(),
      cameraImage.cropRect.top.toDouble(),
      cameraImage.cropRect.right.toDouble(),
      cameraImage.cropRect.bottom.toDouble(),
    );
    final s = min(size.width, size.height) / cameraImage.height;
    final o =
        (max(size.width, size.height) - (cameraImage.width * s).toInt()) / 2;
    canvas.save();

    if (cameraImage.rotationDegrees == 90) {
      canvas.translate(size.width, 0);
      canvas.rotate(_degreesToRadians(cameraImage.rotationDegrees));
    }
    canvas.translate(o, 0);
    canvas.scale(s, s);
    canvas.drawRect(cropRect, rectPaint);
    canvas.restore();
  }

  @override
  bool shouldRepaint(covariant CropRectPainter oldDelegate) {
    return oldDelegate.cameraImage != cameraImage ||
        oldDelegate.rectPaint != rectPaint;
  }

  double _degreesToRadians(num degrees) => degrees * pi / 180;
}
