import 'dart:math';

import 'package:fitatu_barcode_scanner/fitatu_barcode_scanner.dart';
import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:fitatu_barcode_scanner/src/infra/common/preview_overlay.dart';
import 'package:flutter/material.dart';

import '../../scanner_preview_mixin.dart';

class AndroidFitatuScannerPreview extends StatefulWidget {
  const AndroidFitatuScannerPreview({
    super.key,
    required this.onResult,
    required this.options,
    this.onChanged,
    this.onError,
    this.overlayBuilder,
  });

  final ScannerOptions options;
  final ValueChanged<String?> onResult;
  final ScannerErrorCallback? onError;
  final VoidCallback? onChanged;
  final PreviewOverlayBuilder? overlayBuilder;

  @override
  State<AndroidFitatuScannerPreview> createState() => AndroidFitatuScannerPreviewState();
}

class AndroidFitatuScannerPreviewState extends State<AndroidFitatuScannerPreview> with ScannerPreviewMixin {
  late FitatuBarcodeScanner _scanner;

  void setStateIfMounted() {
    if (mounted) {
      setState(() {});
    }
  }

  @override
  void initState() {
    super.initState();
    _scanner = FitatuBarcodeScanner(
      onResult: (code) => widget.onResult(code),
      onError: (e) => widget.onError?.call(e),
    )..addListener(_scannerListener);
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
    final cameraImage = _scanner.cameraImage;

    return Stack(
      fit: StackFit.expand,
      children: [
        if (cameraConfig != null)
          ClipRect(
            child: ConstrainedBox(
              constraints: const BoxConstraints.expand(),
              child: FittedBox(
                fit: BoxFit.cover,
                child: SizedBox.fromSize(
                  size: Size(
                    min(cameraConfig.previewHeight, cameraConfig.previewWidth).toDouble(),
                    max(cameraConfig.previewHeight, cameraConfig.previewWidth).toDouble(),
                  ),
                  child: Texture(
                    key: ValueKey(cameraConfig),
                    textureId: cameraConfig.textureId,
                  ),
                ),
              ),
            ),
          )
        else
          const SizedBox.shrink(),
        if (cameraImage != null)
          Builder(
            builder: (context) {
              final metrix = CameraPreviewMetrix(
                cropRect: Rect.fromLTRB(
                  cameraImage.cropRect.left.toDouble(),
                  cameraImage.cropRect.top.toDouble(),
                  cameraImage.cropRect.right.toDouble(),
                  cameraImage.cropRect.bottom.toDouble(),
                ),
                width: cameraImage.width.toDouble(),
                height: cameraImage.height.toDouble(),
                rotationDegrees: cameraImage.rotationDegrees,
              );

              return widget.overlayBuilder?.call(context, metrix) ??
                  PreviewOverlay(
                    cameraPreviewMetrix: metrix,
                  );
            },
          )
      ],
    );
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    _scanner.setTorchEnabled(isEnabled);
  }

  @override
  bool isTorchEnabled() => _scanner.isTorchEnabled;
}
