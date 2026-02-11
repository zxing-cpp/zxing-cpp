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
    [](const QList<Barcode>& barcodes) {
        for (const auto& barcode : barcodes)
            qDebug() << "Found:" << barcode.text();
    });

QObject::connect(&reader, &BarcodeReader::foundNoBarcodes, 
    []() {
        qDebug() << "No barcodes found";
    });

QImage image("barcode.png");
reader.readAsync(image); // reads barcodes in background thread and emits signals
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
barcode.symbology()         // BarcodeFormat enum (normalized format)
barcode.text()              // Decoded text content
barcode.text(TextMode)      // Decoded text with specific mode (Plain, ECI, HRI, Escaped, Hex, HexECI)
barcode.bytes()             // Raw byte data
barcode.bytesECI()          // Bytes with ECI encoding
barcode.contentType()       // ContentType enum (Text, Binary, Mixed, GS1, ISO15434, UnknownECI)
barcode.hasECI()            // Check if barcode has ECI
barcode.error()             // Error information (type, message, location)
barcode.position()          // Quadrilateral position in image (topLeft, topRight, bottomRight, bottomLeft, center)
barcode.orientation()       // Orientation in degrees
barcode.isMirrored()        // Check if barcode is mirrored
barcode.isInverted()        // Check if barcode is inverted
barcode.symbologyIdentifier() // ISO/IEC 15424 symbology identifier
barcode.sequenceSize()      // Total number of symbols in structured append sequence
barcode.sequenceIndex()     // Index of this symbol in sequence
barcode.sequenceId()        // Parity/checksum data for sequence
barcode.isLastInSequence()  // Check if this is the last symbol in sequence
barcode.isPartOfSequence()  // Check if this is part of a structured append sequence
barcode.extra(key)          // Get extra metadata by key
barcode.lineCount()         // Number of lines (for linear codes)
barcode.symbol()            // Get symbol image (1 pixel per module)
barcode.toImage(options)    // Convert to QImage (for writing)
barcode.toSVG(options)      // Export as SVG (for writing)
```

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
BarcodeFormat::AllMatrix    // All 2D barcodes (without the stacked ones)
BarcodeFormat::AllReadable
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

## BarcodeReader Properties

The `BarcodeReader` class provides the following configurable properties:

```cpp
reader.setFormats(formats)       // BarcodeFormats to scan for
reader.setTextMode(mode)         // Text decoding mode (Plain, ECI, HRI, Escaped, Hex, HexECI)
reader.setTryRotate(bool)        // Try rotating image for better detection
reader.setTryHarder(bool)        // Try harder to detect barcodes (slower)
reader.setTryInvert(bool)        // Try inverting colors
reader.setTryDownscale(bool)     // Try downscaling large images
reader.setIsPure(bool)           // Assume image contains only a pure barcode
reader.setReturnErrors(bool)     // Return failed reads with error info
reader.setMaxThreadCount(int)    // Maximum worker threads for async processing
```

## Performance Tips

1. **Limit formats**: Only enable formats you actually need
2. **Avoid tryHarder**: Use only when necessary (significantly slower)
3. **Threading**: The `BarcodeReader` class can process input asynchronously (see also `maxThreadCount`)
4. **Frame skipping**: When processing video frames and all threads from the pool are busy, the frame is dropped

## Building

Add to your Qt project:

```qmake
# .pro file
INCLUDEPATH += path/to/zxing-cpp-install-root/include
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

To build the demo programs like ZXingQtCamReader, set cmake config variable `ZXING_EXAMPLES_QT=ON`.

**Note on Qt Meta-Object Compiler (moc)**: If `ZXingQt.h` is located outside your project directory (e.g., in a system include path), Qt's automoc may not detect the `Q_OBJECT` classes in the header. To fix this, add the following line at the end of your `.cpp` file that uses `BarcodeReader` or other Qt classes from `ZXingQt.h`:

```cpp
#include "moc_ZXingQt.cpp"
```

This explicitly includes the moc-generated file, ensuring proper signal/slot functionality without requiring CMake workarounds.
