#include <iostream>
#include <fstream>
#include "ConfigContext.hpp"
#include <json/json.h>

#include <QFileSystemWatcher>
#include <QApplication>

using namespace std;


int main(int argc, char **argv) {
    QApplication app(argc, argv);

    ConfigContext ctxt;
    ctxt.addFile("example.json");
    ctxt.addFile("example2.json");


    return app.exec();
}
