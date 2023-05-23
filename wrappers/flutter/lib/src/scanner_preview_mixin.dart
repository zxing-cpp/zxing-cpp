import 'package:flutter/widgets.dart';

mixin ScannerPreviewMixin<T extends StatefulWidget> on State<T> {
  void setTorchEnabled({required bool isEnabled});
  bool isTorchEnabled();
}
