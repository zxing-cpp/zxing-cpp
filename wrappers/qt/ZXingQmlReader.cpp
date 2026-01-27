/*
 * Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ZXingQt.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	QGuiApplication app(argc, argv);
	app.setApplicationName(QStringLiteral("ZXingQtCamReader"));
	QQmlApplicationEngine engine;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	engine.load(QUrl(QStringLiteral("qrc:/ZXingQml5Reader.qml")));
#else
	engine.load(QUrl(QStringLiteral("qrc:/ZXingQml6Reader.qml")));
#endif
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
