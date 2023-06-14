import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import '../../pigeon.dart';

import 'android/android_fitatu_scanner_preview.dart';
import 'common/common_fitatu_scanner_preview.dart';
import '../scanner_preview_mixin.dart';
import 'android/camera_permissions_guard.dart';

class FitatuBarcodeScannerPreview extends StatefulWidget {
  const FitatuBarcodeScannerPreview({
    super.key,
    required this.onSuccess,
    required this.options,
    this.alwaysUseCommon = false,
    this.onChanged,
  });

  final ScannerOptions options;
  final ValueChanged<String> onSuccess;
  final bool alwaysUseCommon;
  final VoidCallback? onChanged;

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
          onChanged: widget.onChanged,
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
          onChanged: widget.onChanged,
        ),
      );
    }

    return getCommonScanner();
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    _key.currentState?.setTorchEnabled(isEnabled: isEnabled);
  }

  @override
  bool isTorchEnabled() => _key.currentState?.isTorchEnabled() ?? false;
}