import 'package:flutter/material.dart';

class CommonScannerError extends StatelessWidget {
  const CommonScannerError({super.key});

  @override
  Widget build(BuildContext context) {
    return const ColoredBox(
      color: Colors.black,
      child: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Padding(
              padding: EdgeInsets.only(bottom: 16),
              child: Icon(Icons.error, color: Colors.white),
            ),
          ],
        ),
      ),
    );
  }
}
