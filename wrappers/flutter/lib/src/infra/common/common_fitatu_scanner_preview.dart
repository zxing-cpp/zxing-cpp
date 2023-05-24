import 'package:camera/camera.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_zxing/flutter_zxing.dart';

import '../../../pigeon.dart';
import '../../scanner_preview_mixin.dart';

class CommonFitatuScannerPreview extends StatefulWidget {
  const CommonFitatuScannerPreview({
    super.key,
    required this.onSuccess,
    required this.options,
    this.onChanged,
  });

  final ValueChanged<String> onSuccess;
  final ScannerOptions options;
  final VoidCallback? onChanged;

  @override
  State<CommonFitatuScannerPreview> createState() =>
      CommonFitatuScannerPreviewState();
}

class CommonFitatuScannerPreviewState extends State<CommonFitatuScannerPreview>
    with ScannerPreviewMixin {
  CameraController? controller;

  void _controllerListener() {
    widget.onChanged?.call();
  }

  @override
  void dispose() {
    controller?.removeListener(_controllerListener);
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Stack(
      fit: StackFit.expand,
      children: [
        ReaderWidget(
          onControllerCreated: (controller) {
            this.controller?.removeListener(_controllerListener);
            this.controller = controller?..addListener(_controllerListener);
          },
          tryHarder: widget.options.tryHarder,
          tryRotate: widget.options.tryRotate,
          tryInverted: widget.options.tryInvert,
          showGallery: false,
          showToggleCamera: false,
          scanDelay: const Duration(milliseconds: 50),
          scanDelaySuccess: const Duration(milliseconds: 300),
          showScannerOverlay: false,
          cropPercent: widget.options.cropPercent,
          resolution: ResolutionPreset.max,
          allowPinchZoom: false,
          showFlashlight: false,
          onScan: (result) {
            final code = result.text;
            if (code != null) {
              widget.onSuccess(code);
            }
          },
        ),
      ],
    );
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    controller?.setFlashMode(
      isEnabled ? FlashMode.torch : FlashMode.off,
    );
  }

  @override
  bool isTorchEnabled() => controller?.value.flashMode == FlashMode.torch;
}
