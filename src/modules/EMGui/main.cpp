/*
 * EMGui
 *
 * Author: Waldez Gomes
 * email:  waldezjr14@gmail.com
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWidgets/QApplication>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>
#include <iostream>
#include <QDir>
#include "emgui.h"

using namespace std;
using namespace yarp::os;
using namespace yarp::sig;

int main(int argc, char *argv[])
{

    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError("YARP server not available!");
        return -1;
    }

    //C++ interface between GUI and the other yarp modules
    EmGui* emgUi = new EmGui();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    //disconnect yarp ports when closing the GUI
    QObject::connect(&engine, &QQmlApplicationEngine::destroyed, emgUi, &EmGui::close);

    //set context property to use EmGui from within QML
    engine.rootContext()->setContextProperty("emgUi", emgUi);

//    std::cout<<endl<<"[INFO]"<<emgUi->rate()<<endl;
    return app.exec();
}
