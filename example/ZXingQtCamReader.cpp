/*
 * Copyright 2020 Axel Waggershauser
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
