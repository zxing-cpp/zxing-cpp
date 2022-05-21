/*
 * Copyright 2020 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "ZXingQtReader.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	ZXingQt::registerQmlAndMetaTypes();

    QGuiApplication app(argc, argv);
    app.setApplicationName("ZXingQtCamReader");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/ZXingQtCamReader.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
