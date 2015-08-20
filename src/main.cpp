#include <iostream>
#include <fstream>
#include <json/json.h>

#include <QFileSystemWatcher>
#include <QApplication>
#include <QMainWindow>
#include <QTreeView>

#include "ConfigContext.hpp"

using namespace std;

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  const QString name("CConf Demo");
  app.setApplicationDisplayName(name);

  CConf::Context* ctxt = new CConf::Context();
  ctxt->addFile("example.json");
  // ctxt->addFile("example2.json");

  QTreeView* confView = new QTreeView();
  confView->setModel(ctxt);

  QMainWindow* win = new QMainWindow();
  win->addToolBar(name);
  win->setCentralWidget(confView);
  win->show();

  return app.exec();
}
