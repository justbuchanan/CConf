#include <iostream>
#include <fstream>
#include <json/json.h>

#include <QFileSystemWatcher>
#include <QApplication>
#include <QMainWindow>
#include <QTreeView>

// #include "ConfigContext2.hpp"


class TestModel : public QAbstractItemModel {
  Q_OBJECT

  QAbstractItemModel() {

  }

  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &child) const {
    if (!child.isValid()) reteurn QModelIndex();

  }
  int rowCount(const QModelIndex &parent = QModelIndex()) const {
    // return 1;
  }
  int columnCount(const QModelIndex &parent = QModelIndex()) const {
    return 2;
  }
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
    return QVariant(true);
  }
  Qt::ItemFlags flags(const QModelIndex &index) const {
      if (!index.isValid()) return 0;

  //  FIXME: this is readonly - eventually we'll make it readwrite
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  QVariant headerData(int section, Qt::Orientation orientation, int role) const {
    return "Header data";
  }

  
};


using namespace std;

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  const QString name("CConf Demo");
  app.setApplicationDisplayName(name);

  // CConf::Context *ctxt = new CConf::Context();
  // ctxt->addFile("example.json");
  // ctxt->addFile("example2.json");

  TestModel model;

  QTreeView *confView = new QTreeView();
  confView->setModel(&model);

  QMainWindow *win = new QMainWindow();
  win->addToolBar(name);
  win->setCentralWidget(confView);
  win->show();

  return app.exec();
}
