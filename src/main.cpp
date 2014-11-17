#include <iostream>
#include <fstream>
#include "ConfigContext.hpp"
#include <json/json.h>

#include <QFileSystemWatcher>
#include <QApplication>


using namespace std;



// typedef enum NodeType {
//     Dictionary,
//     Bool,
//     Integer,
//     String,
//     Array
// };


// void merge(Json::Value &out, const vector<Json::Value&> inTrees) {
//     for (string key in inTrees) {

//     }
// }


void fileChanged(const QString &path) {
    cout << "File Changed: " << path.toStdString() << endl;
}

void directoryChanged(const QString &path) {
    cout << "Directory Changed: " << path.toStdString() << endl;
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    ConfigContext ctxt;

    ctxt.addFile("example.json");
    ctxt.addFile("example2.json");

    // ConfigTree tree("example.json");
    // (*tree.jsonValue())["val"].setComment("// My comment", Json::commentAfterOnSameLine);

    // cout << "Value: " << endl << *tree.jsonValue();


    // Json::Value jsonRoot;
    // // jsonRoot[0].setComment("justin", Json::commentAfterOnSameLine);
    // // jsonRoot["val"].setComment("// justin", Json::commentAfterOnSameLine);
    // // jsonRoot["val"] = 4;
    // jsonRoot["val"]["abc"] = 2;

    // jsonRoot["val"]["abc"].setComment("// justin", Json::commentAfterOnSameLine);


    // cout << "Value: " << endl << jsonRoot;

    // ofstream out("out.json");
    // // jsonRoot.write(out);

    // Json::StyledWriter writer;
    // out << writer.write(jsonRoot);
    // out.flush();



    return app.exec();
}
