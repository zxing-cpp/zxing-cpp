# ZXing-C++ Qt Wrapper

A header-only Qt wrapper for the ZXing-C++ barcode library, providing idiomatic Qt APIs for barcode reading and writing.

## Features

- **Header-only**: Just include `ZXingQt.h`
- **Qt-style API**: Uses Qt types (`QString`, `QImage`, `QByteArray`, etc.)
- **Multiple input formats**: Read from `QImage`, `QVideoFrame`
- **QML integration**: Ready-to-use QML components
- **Signal/slot support**: Asynchronous barcode reading with Qt signals
- **Video streaming**: Built-in support for real-time camera processing

## Basic Usage

### Reading Barcodes

#### Simple Function API

```cpp
#include "ZXingQt.h"

using namespace ZXingQt;

// Read from QImage
for (const auto& barcode : ReadBarcodes(QImage("input.png"))) {
    qDebug() << "Text:" << barcode.text();
    qDebug() << "Format:" << barcode.format();
}
```

#### QObject-based API with Signals

```cpp
BarcodeReader reader;
reader.setFormats({BarcodeFormat::QRCode, BarcodeFormat::DataMatrix});
reader.setTryRotate(true);
reader.setTryInvert(false);
reader.setTextMode(TextMode::HRI);

QObject::connect(&reader, &BarcodeReader::foundBarcodes, 
    [](const QVector<Barcode>& barcodes) {
        for (const auto& barcode : barcodes)
            qDebug() << "Found:" << barcode.text();
    });

QObject::connect(&reader, &BarcodeReader::foundNoBarcodes, 
    []() {
        qDebug() << "No barcodes found";
    });

// Read synchronously (emits signals)
QImage image("barcode.png");
reader.read(image);
```

### Writing Barcodes

#### Create Barcode from Text

```cpp
using namespace ZXingQt;

// Simple creation
auto barcode = Barcode::fromText("Hello World", BarcodeFormat::QRCode, "ecLevel=H");

// Save as image
QImage image = barcode.toImage(WriterOptions().scale(4));
image.save("qrcode.png");

// Export as SVG
QString svg = barcode.toSVG();
```

#### Create Barcode from Binary Data

```cpp
QByteArray data = getMyBinaryData();
auto barcode = Barcode::fromBytes(data, BarcodeFormat::DataMatrix);
barcode.toImage().save("datamatrix.png");
```

### Working with Video Streams

#### Qt 6 Multimedia Integration

```cpp
#include <QVideoSink>
#include <QMediaCaptureSession>

BarcodeReader reader;
reader.setFormats({BarcodeFormat::AllReadable});

connect(&reader, &BarcodeReader::foundBarcodes, this, &MyClass::onBarcodesFound);

// Connect to video sink
QVideoSink* videoSink = new QVideoSink(this);
reader.setVideoSink(videoSink);

// Set up camera
QMediaCaptureSession session;
session.setVideoOutput(videoSink);
// ... configure camera and start
```

See `ZXingQtCamReader.cpp` for a complete camera reader example with GUI.

## Barcode Properties

The `Barcode` class provides:

```cpp
barcode.isValid()           // Check if barcode was successfully read
barcode.format()            // BarcodeFormat enum
barcode.text()              // Decoded text content
barcode.bytes()             // Raw byte data
barcode.contentType()       // ContentType enum
barcode.position()          // Quadrilateral position in image
barcode.toImage()           // Convert to QImage (for writing)
barcode.toSVG()             // Export as SVG (for writing)
```

This is only a subset of all available, more are coming...

## Barcode Formats

Use the `BarcodeFormat` enum for format specification:

```cpp
BarcodeFormat::QRCode
BarcodeFormat::DataMatrix
BarcodeFormat::EAN13
BarcodeFormat::Code128
BarcodeFormat::PDF417
BarcodeFormat::Aztec
// ... and many more

// Special groups, usable when reading barcodes
BarcodeFormat::AllLinear    // All 1D barcodes
BarcodeFormat::AllMatrix    // All 2D barcodes
BarcodeFormat::AllReadable  // All supported formats
// ... and more
```

To and from string conversion:

```cpp
BarcodeFormat format = BarcodeFormatFromString("qrcode"); // case insensitive
QString name = ToString(format); // == "QR Code"
```

## QML Integration

Make sure `QT_QML_LIB` is defined when including `ZXingQt.h` to enable and automatically registering the QML support.

```qml
import ZXing 1.0

Item {
    BarcodeReader {
        id: reader
        formats: [ZXing.QRCode, ZXing.DataMatrix]
        tryRotate: true
        
        onFoundBarcodes: function(barcodes) {
            console.log("Found", barcodes.length, "barcode(s)")
            console.log("First text:", barcodes[0].text)
        }
        
        onFoundNoBarcodes: {
            console.log("No barcodes found")
        }
    }
    
    // Connect to video source
    Camera {
        id: camera
    }
    
    VideoOutput {
        source: camera
        Component.onCompleted: {
            reader.videoSink = videoSink
        }
    }
}
```

See the `ZXingQmlReader` example program.

## Performance Tips

1. **Limit formats**: Only enable formats you actually need
2. **Avoid tryHarder**: Use only when necessary (significantly slower)
3. **Threading**: The `BarcodeReader` class handles threading automatically with `maxThreadCount`
4. **Frame skipping**: When processing video frames and all threads from the pool are busy, the frame is dropped

## Building

Add to your Qt project:

```qmake
# .pro file
INCLUDEPATH += path/to/zxing-cpp/core/src
INCLUDEPATH += path/to/zxing-cpp/wrappers/qt
LIBS += -lZXing  # or link statically

# Enable multimedia support
QT += multimedia
```

Or with CMake:

```cmake
find_package(Qt6 COMPONENTS Core Gui Multimedia REQUIRED)
find_package(ZXing REQUIRED)

target_link_libraries(myapp 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Multimedia
    ZXing::ZXing
)
```
