import 'package:flutter/widgets.dart';
import 'package:permission_handler/permission_handler.dart';

final _hasPermissions = ValueNotifier<bool>(false);

class CameraPermissionsGuard extends StatefulWidget {
  const CameraPermissionsGuard({
    super.key,
    required this.child,
    this.placeholder = const SizedBox.shrink(),
  });

  final Widget child;
  final Widget placeholder;

  @override
  State<CameraPermissionsGuard> createState() => _CameraPermissionsGuardState();
}

class _CameraPermissionsGuardState extends State<CameraPermissionsGuard> {
  @override
  void initState() {
    checkPermissions();
    super.initState();
  }

  Future<void> checkPermissions() async {
    _hasPermissions.value = await Permission.camera.isGranted
        ? true
        : await Permission.camera
            .request()
            .then((value) => value == PermissionStatus.granted);
  }

  @override
  Widget build(BuildContext context) {
    return ValueListenableBuilder(
      valueListenable: _hasPermissions,
      builder: (context, value, child) {
        if (value) {
          return widget.child;
        }
        return widget.placeholder;
      },
    );
  }
}
