#pragma once

#include <json/json.h>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <QFileSystemWatcher>
#include <QAbstractItemModel>

using namespace std;




class TreePath {
public:
    /// @param path A dot-separated key path
    TreePath(string path);

private:
    vector<string> _components;
};






class ConfigContext : public QObject {
    Q_OBJECT

public:
    ConfigContext() {
        QObject::connect(&_fsWatcher, &QFileSystemWatcher::fileChanged, this, &ConfigContext::fileChanged);
    }

    const Json::Value &jsonValue() {
        return _combined;
    }

    void fileChanged(const QString &filePath) {
        cout << "ConfigContext file changed: " << filePath.toStdString() << endl;
    }

    void addFile(const string &path) {
        if (containsFile(path)) {
            throw "this config file is already loaded";
        }


        auto tree = readFile(path);
        if (!tree) {
            throw "Unable to load file";
        }

        _configTrees.push_back(pair<string, shared_ptr<Json::Value>>(path, tree));
        _fsWatcher.addPath(QString::fromStdString(path));
    }

    bool containsFile(const string &path) {
        return indexOfFile(path) != -1;
    }

    shared_ptr<Json::Value> readFile(string filePath) {
        ifstream doc(filePath);
        shared_ptr<Json::Value> root(new Json::Value());
        Json::Reader reader;
        if (!reader.parse(doc, *root)) {
            // throw std::exception("failed to parse json: " + reader.getFormattedErrorMessages());
            return nullptr;
        }

        return root;
    }

    void removeFile(const string &filePath) {
        int idx = indexOfFile(filePath);
        if (idx == -1) {
            cerr << "Warning: Attempt to remove file from ConfigContext that's not in the context: " << filePath;
        } else {
            _configTrees.erase(_configTrees.begin() + idx);
            _fsWatcher.removePath(QString::fromStdString(filePath));
        }
    }


    /// returns -1 if the file isn't a part of this context
    int indexOfFile(const string &path) {
        for (int i = 0; i < _configTrees.size(); i++) {
            if (_configTrees[i].first == path) return i;
        }

        return -1;
    }


private:
    //  higher index = higher precedence when cascading values
    vector<pair<string, shared_ptr<Json::Value>>> _configTrees;
    Json::Value _combined;
    QFileSystemWatcher _fsWatcher;
};





template<typename T>
class ConfigValue {
public:
    ConfigValue(string keyPath, T defaultValue, string comment = "");
    ConfigValue(shared_ptr<ConfigContext> ctxt, string keyPath, T defaultValue, string comment = "");

    void setContext(ConfigContext *ctxt) {
        _context = ctxt;
    }

    ConfigContext context() {
        return _context;
    }

    const string &comment() const {
        return _comment;
    }


    /**
     * The file to write newly-inserted values to.
     * 
     * @param writeToFile the file path
     */
    void setWriteToFile(string writeToFile);


protected:
    friend class ConfigContext;

    void valueChanged(T newValue);

private:
    string _keyPath;
    T _value;
    vector<string> _scope;
    string _comment;
    shared_ptr<ConfigContext> _context;
};





class ConfigDouble : public ConfigValue<double> {
public:
    ConfigDouble(string keyPath, double defaultValue = 0, string comment = "");
    operator double();
};
