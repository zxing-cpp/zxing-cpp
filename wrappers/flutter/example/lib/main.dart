import 'dart:io';

import 'package:fitatu_barcode_scanner/pigeon.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

import 'package:fitatu_barcode_scanner/fitatu_barcode_scanner.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
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
  bool fullscreen = false;
  double cropPercent = 0.8;
  String? code;
  String? error;
  late final _previewKey = GlobalKey<FitatuBarcodeScannerPreviewState>();

  @override
  Widget build(BuildContext context) {
    final isAndroid = !kIsWeb && Platform.isAndroid;
    final options = ScannerOptions(
      tryHarder: tryHarder,
      tryRotate: tryRotate,
      tryInvert: tryInvert,
      qrCode: qrCode,
      cropPercent: cropPercent,
      scanDelay: 50,
      scanDelaySuccess: 300,
    );
    final preview = Material(
      child: Stack(
        children: [
          FitatuBarcodeScannerPreview(
            key: _previewKey,
            options: options,
            alwaysUseCommon: alwaysUseCommon,
            onResult: (value) {
              setState(() {
                code = value;
                error = null;
              });
            },
            onError: (value) {
              setState(() {
                code = null;
                error = value;
              });
            },
            onChanged: () {
              setState(() {
                enableTorch = _previewKey.currentState?.isTorchEnabled() ?? false;
              });
            },
          ),
          Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
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
                alignment: Alignment.topCenter,
                child: Container(
                  margin: const EdgeInsets.all(16),
                  constraints: const BoxConstraints(
                    minWidth: 200,
                    minHeight: 20,
                  ),
                  color: Colors.white.withOpacity(0.5),
                  child: Text(
                    error ?? '<no errors>',
                    style: const TextStyle(color: Colors.black),
                    textAlign: TextAlign.center,
                  ),
                ),
              ),
            ],
          ),
          Align(
            alignment: Alignment.bottomRight,
            child: IconButton(
              onPressed: () {
                setState(() {
                  enableTorch = !enableTorch;
                });
                _previewKey.currentState?.setTorchEnabled(isEnabled: enableTorch);
              },
              icon: Icon(
                enableTorch ? Icons.flash_off : Icons.flash_on,
              ),
            ),
          ),
          Align(
            alignment: Alignment.bottomLeft,
            child: IconButton(
              icon: Icon(fullscreen ? Icons.fullscreen_exit : Icons.fullscreen),
              onPressed: () {
                setState(() {
                  fullscreen = !fullscreen;
                });
              },
            ),
          ),
        ],
      ),
    );

    return MaterialApp(
      theme: ThemeData(
        iconTheme: const IconThemeData(
          color: Colors.white,
        ),
      ),
      home: fullscreen
          ? preview
          : DefaultTabController(
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
                      preview
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
                            onChanged: isAndroid ? (value) => setState(() => alwaysUseCommon = !alwaysUseCommon) : null,
                            subtitle: !isAndroid
                                ? const Text(
                                    'Options is available only on Android device',
                                  )
                                : null,
                          ),
                          SwitchListTile(
                            value: tryHarder,
                            title: const Text('tryHarder'),
                            onChanged: (value) => setState(() => tryHarder = !tryHarder),
                          ),
                          SwitchListTile(
                            value: tryRotate,
                            title: const Text('tryRotate'),
                            onChanged: (value) => setState(() => tryRotate = !tryRotate),
                          ),
                          SwitchListTile(
                            value: tryInvert,
                            title: const Text('tryInvert'),
                            onChanged: (value) => setState(() => tryInvert = !tryInvert),
                          ),
                          SwitchListTile(
                            value: qrCode,
                            title: const Text('qrCode'),
                            onChanged: (value) => setState(() => qrCode = !qrCode),
                          ),
                          Slider(
                            value: cropPercent,
                            max: 1,
                            divisions: 10,
                            label: cropPercent.toString(),
                            onChanged: (value) {
                              if (value < 0.1) return;
                              setState(() {
                                cropPercent = value;
                              });
                            },
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
