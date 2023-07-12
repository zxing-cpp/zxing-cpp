import 'dart:io';

import 'package:fitatu_barcode_scanner/fitatu_barcode_scanner.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import '../../pigeon.dart';

import 'android/android_fitatu_scanner_preview.dart';
import 'common/common_fitatu_scanner_preview.dart';
import '../scanner_preview_mixin.dart';
import 'android/camera_permissions_guard.dart';

typedef PreviewOverlayBuilder = Widget Function(BuildContext context, CameraPreviewMetrix metrix);

class FitatuBarcodeScannerPreview extends StatefulWidget {
  const FitatuBarcodeScannerPreview({
    super.key,
    required this.onResult,
    required this.options,
    this.onError,
    this.onChanged,
    this.alwaysUseCommon = false,
    this.previewOverlayBuilder,
    this.theme = const PreviewOverlayThemeData(),
  });

  final ScannerOptions options;
  final ValueChanged<String?> onResult;
  final ScannerErrorCallback? onError;
  final VoidCallback? onChanged;
  final bool alwaysUseCommon;
  final PreviewOverlayBuilder? previewOverlayBuilder;
  final PreviewOverlayThemeData theme;

  @override
  State<FitatuBarcodeScannerPreview> createState() => FitatuBarcodeScannerPreviewState();
}

class FitatuBarcodeScannerPreviewState extends State<FitatuBarcodeScannerPreview> with ScannerPreviewMixin {
  late final _key = GlobalKey<ScannerPreviewMixin>();

  @override
  Widget build(BuildContext context) {
    late Widget preview;

    Widget getCommonScanner() => CommonFitatuScannerPreview(
          key: _key,
          onResult: widget.onResult,
          options: widget.options,
          onChanged: widget.onChanged,
          onError: widget.onError,
          overlayBuilder: widget.previewOverlayBuilder,
        );

    if (widget.alwaysUseCommon || kIsWeb) {
      preview = getCommonScanner();
    } else if (Platform.isAndroid) {
      preview = CameraPermissionsGuard(
        child: AndroidFitatuScannerPreview(
          key: _key,
          onResult: widget.onResult,
          options: widget.options,
          onChanged: widget.onChanged,
          onError: widget.onError,
          overlayBuilder: widget.previewOverlayBuilder,
        ),
      );
    } else {
      preview = getCommonScanner();
    }

    return PreviewOverlayTheme(themeData: widget.theme, child: preview);
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    _key.currentState?.setTorchEnabled(isEnabled: isEnabled);
  }

  @override
  bool isTorchEnabled() => _key.currentState?.isTorchEnabled() ?? false;
}
