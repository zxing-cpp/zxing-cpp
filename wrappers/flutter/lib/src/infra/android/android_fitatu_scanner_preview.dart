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
    this.overlayBuilder,
  });

  final ScannerOptions options;
  final ValueChanged<String> onSuccess;
  final ValueWidgetBuilder<CameraImage>? overlayBuilder;

  @override
  State<AndroidFitatuScannerPreview> createState() =>
      AndroidFitatuScannerPreviewState();
}

class AndroidFitatuScannerPreviewState
    extends State<AndroidFitatuScannerPreview> with ScannerPreviewMixin {
  late FitatuBarcodeScanner scanner;
  late ValueNotifier<CameraImage?> cameraImage;
  int? textureId;

  void setStateIfMounted() {
    if (mounted) {
      setState(() {});
    }
  }

  @override
  void initState() {
    super.initState();
    cameraImage = ValueNotifier(null);
    scanner = FitatuBarcodeScanner(
      onReady: (textureId) {
        this.textureId = textureId;
        setStateIfMounted();
      },
      onSuccess: widget.onSuccess,
      onCameraImage: (value) => cameraImage.value = value,
    );
    scanner.init(widget.options);
  }

  @override
  void dispose() {
    scanner.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (textureId != null) {
      return Stack(
        fit: StackFit.expand,
        children: [
          Texture(textureId: textureId!),
          ValueListenableBuilder(
            valueListenable: cameraImage,
            builder: (context, value, child) {
              if (value == null || widget.overlayBuilder == null) {
                return const SizedBox.shrink();
              }

              return widget.overlayBuilder!(
                context,
                value,
                CustomPaint(
                  painter: CropRectPainter(
                    value,
                    Paint()
                      ..color = Colors.green
                      ..style = PaintingStyle.stroke
                      ..strokeWidth = 8,
                  ),
                ),
              );
            },
          )
        ],
      );
    }
    return const SizedBox.shrink();
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    scanner.setTorchEnabled(isEnabled);
  }
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
