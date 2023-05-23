import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import '../../pigeon.dart';

import 'android/android_fitatu_scanner_preview.dart';
import 'common/common_fitatu_scanner_preview.dart';
import '../scanner_preview_mixin.dart';
import 'android/camera_permissions_guard.dart';

class FitatuBarcodeScannerPreview extends StatefulWidget {
  FitatuBarcodeScannerPreview({
    super.key,
    required this.onSuccess,
    ScannerOptions? options,
    this.alwaysUseCommon = false,
    this.overlayBuilder,
  }) : options = options ??
            ScannerOptions(
              tryHarder: false,
              tryRotate: true,
              tryInvert: true,
              qrCode: false,
              cropPercent: 0.8,
            );

  final ScannerOptions options;
  final ValueChanged<String> onSuccess;
  final bool alwaysUseCommon;
  final WidgetBuilder? overlayBuilder;

  @override
  State<FitatuBarcodeScannerPreview> createState() =>
      FitatuBarcodeScannerPreviewState();
}

class FitatuBarcodeScannerPreviewState
    extends State<FitatuBarcodeScannerPreview> with ScannerPreviewMixin {
  late final _key = GlobalKey<ScannerPreviewMixin>();

  @override
  Widget build(BuildContext context) {
    Widget getCommonScanner() => CommonFitatuScannerPreview(
          key: _key,
          onSuccess: widget.onSuccess,
          options: widget.options,
          overlayBuilder: widget.overlayBuilder,
        );

    if (widget.alwaysUseCommon || kIsWeb) {
      return getCommonScanner();
    }

    if (Platform.isAndroid) {
      return CameraPermissionsGuard(
        child: AndroidFitatuScannerPreview(
          key: _key,
          onSuccess: widget.onSuccess,
          options: widget.options,
          overlayBuilder: (context, value, child) {
            return widget.overlayBuilder?.call(context) ??
                const SizedBox.shrink();
          },
        ),
      );
    }

    return getCommonScanner();
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    _key.currentState?.setTorchEnabled(isEnabled: isEnabled);
  }
}
