/*
 * Copyright 2026 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include "ZXingQt.h"

#include <QApplication>
#include <QCamera>
#include <QCameraDevice>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDebug>
#include <QElapsedTimer>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QVideoFrame>
#include <QVideoSink>
#include <QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QPermission>
#endif

using namespace ZXingQt;

class VideoWidget : public QWidget
{
	Q_OBJECT

public:
	VideoWidget(QWidget* parent = nullptr) : QWidget(parent)
	{
		setAttribute(Qt::WA_OpaquePaintEvent);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	void setVideoFrame(const QVideoFrame& frame)
	{
		_frame = frame;
		update();
	}

	void setBarcodes(const QList<Barcode>& barcodes)
	{
		_barcodes = barcodes;
		update();
	}

protected:
	void paintEvent(QPaintEvent*) override
	{
		QPainter painter(this);
		painter.fillRect(rect(), Qt::black);

		if (!_frame.isValid())
			return;

		auto image = _frame.toImage();
		if (image.isNull())
			return;

		// Scale image to fit widget while maintaining aspect ratio
		QSize imageSize = image.size();
		imageSize.scale(size(), Qt::KeepAspectRatio);

		int x = (width() - imageSize.width()) / 2;
		int y = (height() - imageSize.height()) / 2;

		QRect targetRect(x, y, imageSize.width(), imageSize.height());
		painter.drawImage(targetRect, image);

		// Draw all barcode positions
		if (!_barcodes.isEmpty() && imageSize.width() > 0) {
			double scaleX = static_cast<double>(imageSize.width()) / image.width();
			double scaleY = static_cast<double>(imageSize.height()) / image.height();

			auto mapPoint = [&](const QPoint& p) { return QPointF(x + p.x() * scaleX, y + p.y() * scaleY); };

			QFont font = painter.font();
			QFontMetrics fm(font);
			font.setPointSize(16);
			font.setBold(true);
			painter.setFont(font);

			int barcodeIndex = 1;
			for (const auto& barcode : _barcodes) {
				const auto& position = barcode.position();
				painter.setPen(QPen(barcode.isValid() ? Qt::green : Qt::red, 3));
				for (int i = 0; i < 4; ++i)
					painter.drawLine(mapPoint(position[i]), mapPoint(position[(i + 1) % 4]));

				if (_barcodes.size() == 1)
					break;

				// Draw barcode index at center
				QString indexText = QString::number(barcodeIndex);
				QRect textRect = fm.boundingRect(indexText);
				textRect.moveCenter(mapPoint(position.center()).toPoint());
				textRect.adjust(-5, -5, 5, 5);

				painter.fillRect(textRect, QColor(128, 128, 128, 180));
				painter.setPen(Qt::white);
				painter.drawText(textRect, Qt::AlignCenter, indexText);

				++barcodeIndex;
			}
		}
	}

private:
	QVideoFrame _frame;
	QList<Barcode> _barcodes;
};

class CameraReaderWidget : public QMainWindow
{
	Q_OBJECT

public:
	CameraReaderWidget(QWidget* parent = nullptr) : QMainWindow(parent)
	{
		setWindowTitle(QStringLiteral("ZXingQtCamReader"));
		resize(640, 480);

		setupUI();
		setupCameraAndReader();
	}

	~CameraReaderWidget() override { _camera->stop(); }

private:
	void keyPressEvent(QKeyEvent* event) override
	{
		if (event->key() == Qt::Key_Space || event->key() == Qt::Key_P) {
			togglePause();
		} else if (event->key() == Qt::Key_S) {
			toggleSettings();
		} else {
			QMainWindow::keyPressEvent(event);
		}
	}

	void setupUI()
	{
		auto centralWidget = new QWidget(this);
		setCentralWidget(centralWidget);

		auto mainLayout = new QVBoxLayout(centralWidget);
		mainLayout->setContentsMargins(0, 0, 0, 0);
		mainLayout->setSpacing(0);

		// Camera selection (only shown when multiple cameras available)
		const auto cameras = QMediaDevices::videoInputs();
		if (cameras.size() > 1) {
			auto cameraLayout = new QHBoxLayout();
			cameraLayout->addWidget(new QLabel(tr("Camera: ")));

			_cameraCombo = new QComboBox();
			for (const auto& camera : cameras)
				_cameraCombo->addItem(camera.description(), QVariant::fromValue(camera));
			connect(_cameraCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraReaderWidget::onCameraChanged);
			cameraLayout->addWidget(_cameraCombo);
			cameraLayout->addStretch();

			mainLayout->addLayout(cameraLayout);
		}

		// Video display with overlays
		_videoWidget = new VideoWidget();

		// Create a container for video widget with overlays using a grid layout
		auto videoContainer = new QWidget();
		auto gridLayout = new QGridLayout(videoContainer);
		gridLayout->setContentsMargins(0, 0, 0, 0);
		gridLayout->addWidget(_videoWidget, 0, 0, 1, 1);

		mainLayout->addWidget(videoContainer, 1);

		// Info label overlay (top-left aligned)
		_infoLabel = new QLabel(videoContainer);
		_infoLabel->setStyleSheet(QStringLiteral(
			"QLabel { color: white; background-color: rgba(40, 40, 40, 200); padding: 12px 16px; border-radius: 8px; }"));
		_infoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
		_infoLabel->setText(tr("Initializing camera..."));
		_infoLabel->adjustSize();
		gridLayout->addWidget(_infoLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignLeft);

		// Feedback label overlay (center aligned)
		_feedbackLabel = new QLabel(videoContainer);
		_feedbackLabel->setStyleSheet(QStringLiteral(
			"QLabel { color: white; background-color: rgba(40, 40, 40, 200); padding: 12px 16px; border-radius: 8px; }"));
		_feedbackLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
		_feedbackLabel->hide();
		gridLayout->addWidget(_feedbackLabel, 0, 0, 1, 1, Qt::AlignCenter);

		// Control buttons container (bottom-right aligned)
		auto buttonContainer = new QWidget(videoContainer);
		auto buttonLayout = new QHBoxLayout(buttonContainer);
		buttonLayout->setContentsMargins(0, 0, 0, 0);
		buttonLayout->setSpacing(8);

		const QString buttonStyle = QStringLiteral("QPushButton { "
												   "  background-color: rgba(40, 40, 40, 200); "
												   "  color: white; "
												   "  border: none; "
												   "  border-radius: 24px; "
												   "  font-size: 24px; "
												   "} "
												   "QPushButton:hover { background-color: rgba(60, 60, 60, 220); } "
												   "QPushButton:pressed { background-color: rgba(80, 80, 80, 240); }");

		// Pause/Resume button
		_pauseButton = new QPushButton(QStringLiteral("⏸"), buttonContainer);
		_pauseButton->setFixedSize(48, 48);
		_pauseButton->setStyleSheet(buttonStyle);
		_pauseButton->setCursor(Qt::PointingHandCursor);
		_pauseButton->setToolTip(tr("Pause capture"));
		connect(_pauseButton, &QPushButton::clicked, this, &CameraReaderWidget::togglePause);
		buttonLayout->addWidget(_pauseButton);

		// Settings button
		_settingsButton = new QPushButton(QStringLiteral("⚙"), buttonContainer);
		_settingsButton->setFixedSize(48, 48);
		_settingsButton->setStyleSheet(buttonStyle);
		_settingsButton->setCursor(Qt::PointingHandCursor);
		_settingsButton->setToolTip(tr("Settings"));
		connect(_settingsButton, &QPushButton::clicked, this, &CameraReaderWidget::toggleSettings);
		buttonLayout->addWidget(_settingsButton);

		gridLayout->addWidget(buttonContainer, 0, 0, 1, 1, Qt::AlignBottom | Qt::AlignRight);

		// Controls panel (initially hidden)
		_controlsWidget = new QWidget(videoContainer);
		_controlsWidget->setStyleSheet(QStringLiteral("QWidget#controlsWidget { "
													  "  background-color: rgba(40, 40, 40, 230); "
													  "  border-radius: 12px; "
													  "} "
													  "QCheckBox, QLabel { "
													  "  color: white; "
													  "  background: transparent; "
													  "  font-size: 13px; "
													  "}"));
		_controlsWidget->setObjectName(QStringLiteral("controlsWidget"));
		_controlsWidget->hide();
		auto controlsLayout = new QVBoxLayout(_controlsWidget);
		controlsLayout->setContentsMargins(16, 16, 16, 16);
		controlsLayout->setSpacing(8);

		// Header with close button
		auto headerLayout = new QHBoxLayout();
		headerLayout->addWidget(new QLabel(tr("Settings")));
		headerLayout->addStretch();

		auto closeButton = new QPushButton(QStringLiteral("✕"));
		closeButton->setFixedSize(24, 24);
		closeButton->setStyleSheet(
			QStringLiteral("QPushButton { background: transparent; border: none; color: white; font-size: 18px; }"));
		closeButton->setCursor(Qt::PointingHandCursor);
		connect(closeButton, &QPushButton::clicked, _controlsWidget, &QWidget::hide);
		headerLayout->addWidget(closeButton);
		controlsLayout->addLayout(headerLayout);

		auto addCheckBox = [&](QCheckBox*& checkbox, const QString& text, bool defaultValue = true) {
			checkbox = new QCheckBox(text);
			checkbox->setChecked(defaultValue);
			connect(checkbox, &QCheckBox::toggled, this, &CameraReaderWidget::updateReaderOptions);
			controlsLayout->addWidget(checkbox);
		};

		addCheckBox(_autoPauseCheck, tr("Auto Pause"), false);
		addCheckBox(_tryRotateCheck, tr("Try Rotate"));
		addCheckBox(_tryHarderCheck, tr("Try Harder"));
		addCheckBox(_tryInvertCheck, tr("Try Invert"));
		addCheckBox(_tryDownscaleCheck, tr("Try Downscale"));
		addCheckBox(_returnErrorsCheck, tr("Return Errors"), false);

		// Format filter combobox
		_formatCombo = new QComboBox();
		for (auto format : ListBarcodeFormats())
			_formatCombo->addItem(ToString(format), static_cast<unsigned int>(format));

		connect(_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CameraReaderWidget::updateReaderOptions);
		controlsLayout->addWidget(_formatCombo);

		// Position controls panel above the settings button with some margin
		gridLayout->addWidget(_controlsWidget, 0, 0, 1, 1, Qt::AlignBottom | Qt::AlignRight);
	}

	void toggleSettings()
	{
		if (_controlsWidget->isVisible()) {
			_controlsWidget->hide();
		} else {
			_controlsWidget->show();
			_controlsWidget->adjustSize();
		}
	}

	void togglePause() { _isPaused ? resumeScan() : pauseScan(); }

	void pauseScan()
	{
		_isPaused = true;
		_barcodeReader->setVideoSink(nullptr);
		disconnect(_videoSink, &QVideoSink::videoFrameChanged, _videoWidget, &VideoWidget::setVideoFrame);
		_pauseButton->setText(QStringLiteral("▶"));
		_pauseButton->setToolTip(tr("Resume capture"));
		_resetTimer->stop();

		if (!_lastBarcodeText.isEmpty()) {
			QApplication::beep();
			QApplication::clipboard()->setText(_lastBarcodeText);
			showFeedback(tr("Copied content of (first) barcode to clipboard."));
		}
	}

	void resumeScan()
	{
		_isPaused = false;
		_barcodeReader->setVideoSink(_videoSink);
		connect(_videoSink, &QVideoSink::videoFrameChanged, _videoWidget, &VideoWidget::setVideoFrame);
		_pauseButton->setText(QStringLiteral("⏸"));
		_pauseButton->setToolTip(tr("Pause capture"));
		_resetTimer->start();
	}

	void setupCameraAndReader()
	{
		// Create camera and video pipeline
		_camera = new QCamera(this);
		_camera->setFocusMode(QCamera::FocusModeAutoNear);

		connect(_camera, &QCamera::errorOccurred, this, [this](QCamera::Error error, const QString& errorString) {
			qWarning() << "Camera error:" << error << errorString;
			_infoLabel->setText(tr("Camera error: %1").arg(errorString));
		});

		connect(_camera, &QCamera::activeChanged, this, [](bool active) { qDebug() << "Camera active state changed:" << active; });

		_captureSession = new QMediaCaptureSession(this);
		_captureSession->setCamera(_camera);

		_videoSink = new QVideoSink(this);
		_captureSession->setVideoOutput(_videoSink);

		connect(_videoSink, &QVideoSink::videoFrameChanged, _videoWidget, &VideoWidget::setVideoFrame);

		if (QMediaDevices::videoInputs().isEmpty()) {
			_infoLabel->setText(tr("No camera found"));
			return;
		}

		_camera->setCameraDevice(QMediaDevices::videoInputs().first());

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
		// Request camera permission for Qt 6.6+
		QCameraPermission cameraPermission;
		switch (qApp->checkPermission(cameraPermission)) {
		case Qt::PermissionStatus::Undetermined:
			qApp->requestPermission(cameraPermission, this, [this](const QPermission& permission) {
				if (permission.status() == Qt::PermissionStatus::Granted) {
					_infoLabel->setText(tr("Starting camera..."));
					_camera->start();
				} else {
					_infoLabel->setText(tr("Camera permission denied"));
				}
			});
			break;
		case Qt::PermissionStatus::Granted:
			_infoLabel->setText(tr("Starting camera..."));
			_camera->start();
			break;
		case Qt::PermissionStatus::Denied: _infoLabel->setText(tr("Camera permission denied")); break;
		}
#else
		_camera->start();
#endif

		// Create barcode reader and configure
		_barcodeReader = new BarcodeReader(this);
		_barcodeReader->setVideoSink(_videoSink);

		connect(_barcodeReader, &BarcodeReader::foundBarcodes, this, &CameraReaderWidget::onBarcodesFound);
		connect(_barcodeReader, &BarcodeReader::foundNoBarcodes, this, &CameraReaderWidget::onFoundNoBarcodes);

		updateReaderOptions();

		_resetTimer = new QTimer(this);
		_resetTimer->setSingleShot(true);
		_resetTimer->setInterval(1000);
		connect(_resetTimer, &QTimer::timeout, [this]() { _infoLabel->clear(); });

		_feedbackTimer = new QTimer(this);
		_feedbackTimer->setSingleShot(true);
		_feedbackTimer->setInterval(2000);
		connect(_feedbackTimer, &QTimer::timeout, [this]() { _feedbackLabel->hide(); });
	}

	void updateReaderOptions()
	{
		_barcodeReader->setFormats({static_cast<BarcodeFormat>(_formatCombo->currentData().toUInt())});
		_barcodeReader->setTryRotate(_tryRotateCheck->isChecked());
		_barcodeReader->setTryHarder(_tryHarderCheck->isChecked());
		_barcodeReader->setTryInvert(_tryInvertCheck->isChecked());
		_barcodeReader->setTryDownscale(_tryDownscaleCheck->isChecked());
		_barcodeReader->setReturnErrors(_returnErrorsCheck->isChecked());
	}

private Q_SLOTS:

	void onBarcodesFound(const QList<Barcode>& barcodes)
	{
		_videoWidget->setBarcodes(barcodes);

		_lastBarcodeText = barcodes.isEmpty() ? QString() : barcodes.first().text();

		// Build info text for all barcodes
		QStringList infoParts;
		for (int i = 0; i < barcodes.size(); ++i) {
			const auto& barcode = barcodes[i];
			if (barcodes.size() > 1)
				infoParts.append(tr("[%1]").arg(i + 1));
			infoParts.append(tr("Format: %1").arg(ToString(barcode.format())));
			if (barcode.isValid())
				infoParts.append(tr("Text: %1").arg(barcode.text()));
			else
				infoParts.append(tr("Error: %1").arg(ToString(barcode.error())));
			infoParts.append(tr("Type: %1").arg(ToString(barcode.contentType())));
			infoParts.append(QStringLiteral(""));
		}
		infoParts.append(tr("Time: %1 ms").arg(_barcodeReader->runTime.loadRelaxed()));

		_infoLabel->setText(infoParts.join(QStringLiteral("\n")));
		_infoLabel->adjustSize();
		if (!_isPaused)
			_resetTimer->start();

		if (_autoPauseCheck->isChecked() && !_isPaused)
			pauseScan();
	}

	void onFoundNoBarcodes()
	{
		_videoWidget->setBarcodes({});

		if (!_resetTimer->isActive() && !_isPaused) {
			_infoLabel->setText(tr("No barcode found (in %1 ms)").arg(_barcodeReader->runTime.loadRelaxed()));
			_infoLabel->adjustSize();
		}
	}

	void onCameraChanged(int index)
	{
		if (index < 0)
			return;

		auto cameraDevice = _cameraCombo->itemData(index).value<QCameraDevice>();
		_camera->stop();
		_camera->setCameraDevice(cameraDevice);
		_camera->start();
	}

	void showFeedback(const QString& message)
	{
		_feedbackLabel->setText(message);
		_feedbackLabel->adjustSize();
		_feedbackLabel->show();
		_feedbackTimer->start();
	}

private:
	VideoWidget* _videoWidget = nullptr;
	QLabel* _infoLabel = nullptr;
	QLabel* _feedbackLabel = nullptr;
	QPushButton* _pauseButton = nullptr;
	QPushButton* _settingsButton = nullptr;
	QWidget* _controlsWidget = nullptr;
	QComboBox* _cameraCombo = nullptr;
	QComboBox* _formatCombo = nullptr;
	QCheckBox* _tryRotateCheck = nullptr;
	QCheckBox* _tryHarderCheck = nullptr;
	QCheckBox* _tryInvertCheck = nullptr;
	QCheckBox* _tryDownscaleCheck = nullptr;
	QCheckBox* _returnErrorsCheck = nullptr;
	QCheckBox* _autoPauseCheck = nullptr;

	QCamera* _camera = nullptr;
	QMediaCaptureSession* _captureSession = nullptr;
	QVideoSink* _videoSink = nullptr;
	BarcodeReader* _barcodeReader = nullptr;
	QTimer* _resetTimer = nullptr;
	QTimer* _feedbackTimer = nullptr;

	bool _isPaused = false;
	QString _lastBarcodeText;
};

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName(QStringLiteral("ZXingQtCamReader"));

	CameraReaderWidget window;
	window.show();

	return app.exec();
}

#include "ZXingQtCamReader.moc"
#include "moc_ZXingQt.cpp"
