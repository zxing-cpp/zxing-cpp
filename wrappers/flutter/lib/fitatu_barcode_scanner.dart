import 'pigeon.dart';

export 'src/fitatu_barcode_scanner.dart';
export 'src/infra/fitatu_barcode_scanner_preview.dart';

extension CropRectExt on CropRect {
  int width() => right - left;

  int height() => bottom - top;
}
