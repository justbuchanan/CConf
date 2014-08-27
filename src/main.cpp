#include <iostream>

#include <QApplication>
#include <QMainWindow>

#include <VarTreeModel.h>
#include <VarTreeView.h>

using namespace std;
using namespace VarTypes;


int main(int argc, char **argv) {
    QApplication app(argc, argv);

    VarTreeModel *model = new VarTreeModel();

    VarTreeView *view = new VarTreeView();
    view->setModel(model);

    QMainWindow *window = new QMainWindow();
    window->setCentralWidget(view);

    window->show();

    return app.exec();
}
