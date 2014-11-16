#include <iostream>
#include <fstream>

#include <QApplication>
#include <QMainWindow>

#include <json/json.h>

using namespace std;


int main(int argc, char **argv) {

    ifstream doc("example.json");

    Json::Value root;
    Json::Reader reader;


    // parse(const std::string& document, Value& root, bool collectComments = true);



    bool success = reader.parse(doc, root, false);
    if (!success) {
        cerr << "Error parsing json: " << reader.getFormattedErrorMessages();
        return EXIT_FAILURE;
    }

    cout << "Value: " << endl << root;

    return 0;
}
