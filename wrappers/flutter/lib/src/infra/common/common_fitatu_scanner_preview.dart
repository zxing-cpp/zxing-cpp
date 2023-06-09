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
  });

  final ValueChanged<String> onSuccess;
  final ScannerOptions options;
  final VoidCallback? onChanged;

  @override
  State<CommonFitatuScannerPreview> createState() => CommonFitatuScannerPreviewState();
}

class CommonFitatuScannerPreviewState extends State<CommonFitatuScannerPreview> with ScannerPreviewMixin {
  final MobileScannerController controller = MobileScannerController();

  @override
  Widget build(BuildContext context) {
    return MobileScanner(
      controller: controller,
      onDetect: (response) {
        final List<Barcode> nonEmptyBarcodes = response.barcodes.where((barcode) => barcode.rawValue != null).toList();
        for (final barcode in nonEmptyBarcodes) {
          widget.onSuccess(barcode.rawValue!);
          return;
        }
      },
    );
  }

  @override
  void setTorchEnabled({required bool isEnabled}) {
    controller.torchState.value = isEnabled ? TorchState.on : TorchState.off;
  }

  @override
  bool isTorchEnabled() => controller.torchState.value == TorchState.on;
}
