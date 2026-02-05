/*
 * Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ZXingQt.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	app.setApplicationName(QStringLiteral("ZXingQtCamReader"));
	QQmlApplicationEngine engine;
	engine.load(QUrl(QStringLiteral("qrc:/ZXingQml6Reader.qml")));
	if (engine.rootObjects().isEmpty())
		return -1;

	return app.exec();
}
