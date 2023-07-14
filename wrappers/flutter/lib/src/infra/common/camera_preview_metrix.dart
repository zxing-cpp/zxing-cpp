import 'package:flutter/services.dart';

class CameraPreviewMetrix {
  final Rect cropRect;
  final double width;
  final double height;
  final int rotationDegrees;

  CameraPreviewMetrix({
    required this.cropRect,
    required this.width,
    required this.height,
    required this.rotationDegrees,
  });

  @override
  int get hashCode => Object.hash(cropRect, width, height, rotationDegrees);

  @override
  bool operator ==(Object other) => other is CameraPreviewMetrix && other.hashCode == hashCode;
}
