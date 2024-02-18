/*
 * Copyright 2020 Axel Waggershauser
 */
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ZXingQtQmlWriter.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QGuiApplication app(argc, argv);
	app.setApplicationName("ZXingQtQmlWriter");

	ZXingQtQmlWriter writer;
	//Registering ZXingQtQmlWriter instance so it is created on QML level.
	//Can be done directly inside CMake using qt_add_qml_module macro
	qmlRegisterType<ZXingQtQmlWriter>("ZXing",1,0, "ZXingQtQmlWriter");

	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/ZXingQtQmlWriter.qml")));
	if (engine.rootObjects().isEmpty()) {
		return -1;
	}

	return app.exec();
}
