#include <iostream>
#include <fstream>
#include <json/json.h>

#include <QFileSystemWatcher>
#include <QApplication>
#include <QMainWindow>

#include "ConfigContext.hpp"
#include "CConfView.hpp"


using namespace std;


int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QString name("CConf Demo");
    app.setApplicationDisplayName(name);


    CConf::Context *ctxt = new CConf::Context();
    ctxt->addFile("example.json");
    ctxt->addFile("example2.json");


    CConfView *confView = new CConfView();
    confView->setModel(ctxt);

    QMainWindow *win = new QMainWindow();
    win->addToolBar(name);
    win->setCentralWidget(confView);
    win->show();


    return app.exec();
}
