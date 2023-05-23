import 'dart:io';

import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import 'package:fitatu_barcode_scanner/fitatu_barcode_scanner.dart';
import 'package:flutter/services.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  SystemChrome.setPreferredOrientations([DeviceOrientation.portraitUp]);
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  bool alwaysUseCommon = kIsWeb || Platform.isIOS;
  bool tryHarder = false;
  bool tryRotate = true;
  bool tryInvert = true;
  bool qrCode = false;
  bool started = false;
  bool enableTorch = false;
  String? code;
  late final _previewKey = GlobalKey<FitatuBarcodeScannerPreviewState>();

  @override
  Widget build(BuildContext context) {
    final isAndroid = !kIsWeb && Platform.isAndroid;
    final options = ScannerOptions(
      tryHarder: tryHarder,
      tryRotate: tryRotate,
      tryInvert: tryInvert,
      qrCode: qrCode,
      cropPercent: 0.8,
    );
    return MaterialApp(
      home: DefaultTabController(
        length: 2,
        child: Scaffold(
          appBar: AppBar(
            title: const Text('FitatuBarcodeScanner example app'),
            bottom: const TabBar(
              tabs: [
                Tab(icon: Icon(Icons.camera)),
                Tab(icon: Icon(Icons.settings)),
              ],
            ),
          ),
          body: TabBarView(
            children: [
              if (started)
                Stack(
                  fit: StackFit.expand,
                  children: [
                    FitatuBarcodeScannerPreview(
                      key: _previewKey,
                      options: options,
                      alwaysUseCommon: alwaysUseCommon,
                      onSuccess: (value) {
                        setState(() {
                          code = value;
                        });
                      },
                    ),
                    Align(
                      alignment: Alignment.topCenter,
                      child: Container(
                        margin: const EdgeInsets.all(16),
                        constraints: const BoxConstraints(
                          minWidth: 200,
                          minHeight: 20,
                        ),
                        color: Colors.white.withOpacity(0.5),
                        child: Text(
                          code ?? '<no results>',
                          style: const TextStyle(color: Colors.black),
                          textAlign: TextAlign.center,
                        ),
                      ),
                    ),
                    Align(
                      alignment: Alignment.bottomRight,
                      child: IconButton(
                        onPressed: () {
                          setState(() {
                            enableTorch = !enableTorch;
                          });
                          _previewKey.currentState
                              ?.setTorchEnabled(isEnabled: enableTorch);
                        },
                        icon: Icon(
                          enableTorch ? Icons.flash_off : Icons.flash_on,
                          color: Colors.black,
                        ),
                      ),
                    ),
                  ],
                )
              else
                MaterialButton(
                  child: const Text('Tap to start'),
                  onPressed: () => setState(() => started = true),
                ),
              SingleChildScrollView(
                child: Column(
                  children: [
                    SwitchListTile(
                      value: alwaysUseCommon,
                      title: const Text('alwaysUseCommon'),
                      onChanged: isAndroid
                          ? (value) =>
                              setState(() => alwaysUseCommon = !alwaysUseCommon)
                          : null,
                      subtitle: !isAndroid
                          ? const Text(
                              'Options is available only on Android device',
                            )
                          : null,
                    ),
                    SwitchListTile(
                      value: tryHarder,
                      title: const Text('tryHarder'),
                      onChanged: (value) =>
                          setState(() => tryHarder = !tryHarder),
                    ),
                    SwitchListTile(
                      value: tryRotate,
                      title: const Text('tryRotate'),
                      onChanged: (value) =>
                          setState(() => tryRotate = !tryRotate),
                    ),
                    SwitchListTile(
                      value: tryInvert,
                      title: const Text('tryInvert'),
                      onChanged: (value) =>
                          setState(() => tryInvert = !tryInvert),
                    ),
                    SwitchListTile(
                      value: qrCode,
                      title: const Text('qrCode'),
                      onChanged: (value) => setState(() => qrCode = !qrCode),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
