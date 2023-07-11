import 'package:fitatu_barcode_scanner/src/infra/common/common_scanner_error.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:mobile_scanner/mobile_scanner.dart';

import '../../../pigeon.dart';
import '../../scanner_preview_mixin.dart';

class CommonFitatuScannerPreview extends StatefulWidget {
  const CommonFitatuScannerPreview({
    super.key,
    required this.onSuccess,
    required this.options,
    this.onChanged,
    this.onError,
  });

  final ValueChanged<String> onSuccess;
  final ScannerOptions options;
  final VoidCallback? onChanged;
  final Function(String error)? onError;

  @override
  State<CommonFitatuScannerPreview> createState() => CommonFitatuScannerPreviewState();
}

class CommonFitatuScannerPreviewState extends State<CommonFitatuScannerPreview> with ScannerPreviewMixin {
  late final MobileScannerController controller;
  String? scannerError;

  @override
  void initState() {
    controller = MobileScannerController();
    controller.hasTorchState.addListener(_torchChangeListener);
    controller.resetZoomScale();

    super.initState();
  }

  @override
  void dispose() {
    controller.hasTorchState.removeListener(_torchChangeListener);
    controller.dispose();
    scannerError = null;

    super.dispose();
  }

  void _handleError(MobileScannerException exception) {
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (scannerError != exception.toString()) {
        widget.onError?.call(exception.toString());
        scannerError = exception.toString();
      }
    });
  }

  void _torchChangeListener() => widget.onChanged?.call();

  @override
  Widget build(BuildContext context) {
    return MobileScanner(
      controller: controller,
      onScannerStarted: (_) async => await controller.resetZoomScale(),
      onDetect: (response) {
        final List<Barcode> nonEmptyBarcodes = response.barcodes.where((barcode) => barcode.rawValue != null).toList();
        for (final barcode in nonEmptyBarcodes) {
          widget.onSuccess(barcode.rawValue!);
          return;
        }
      },
      errorBuilder: (_, exception, __) {
        _handleError(exception);

        return const CommonScannerError();
      },
    );
  }

  @override
  void setTorchEnabled({required bool isEnabled}) => controller.toggleTorch();

  @override
  bool isTorchEnabled() => controller.torchState.value == TorchState.on;
}
