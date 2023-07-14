import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

class PreviewOverlayTheme extends InheritedWidget {
  const PreviewOverlayTheme({
    super.key,
    required this.themeData,
    required super.child,
  });

  final PreviewOverlayThemeData themeData;

  static PreviewOverlayThemeData of(BuildContext context) {
    final theme = context.dependOnInheritedWidgetOfExactType<PreviewOverlayTheme>();
    assert(theme != null, 'Cannot find PreviewOverlayTheme in given context');
    return theme!.themeData;
  }

  @override
  bool updateShouldNotify(covariant InheritedWidget oldWidget) {
    return oldWidget is! PreviewOverlayTheme || oldWidget.themeData != themeData;
  }
}

class PreviewOverlayThemeData {
  final Color overlayColor;
  final Color laserLineColor;
  final double laserLineThickness;
  final bool showLaserLine;
  final List<DeviceOrientation> supportedOrientations;

  const PreviewOverlayThemeData({
    this.overlayColor = const Color(0x80000000),
    this.laserLineColor = const Color(0xFFFF3939),
    this.laserLineThickness = 1.0,
    this.showLaserLine = true,
    this.supportedOrientations = DeviceOrientation.values,
  });

  @override
  int get hashCode => Object.hash(
        overlayColor,
        laserLineColor,
        laserLineThickness,
        showLaserLine,
        supportedOrientations,
      );

  @override
  bool operator ==(Object other) => other is PreviewOverlayThemeData && other.hashCode == hashCode;
}
