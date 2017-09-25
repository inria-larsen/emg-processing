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
