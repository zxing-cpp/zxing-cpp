/*
 * Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ZXingQtReader.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	ZXingQt::registerQmlAndMetaTypes();

	QGuiApplication app(argc, argv);
	app.setApplicationName("ZXingQtCamReader");
	QQmlApplicationEngine engine;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	engine.load(QUrl(QStringLiteral("qrc:/ZXingQt5CamReader.qml")));
#else
	engine.load(QUrl(QStringLiteral("qrc:/ZXingQt6CamReader.qml")));
#endif
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
