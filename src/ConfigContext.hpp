#pragma once

#include <json/json.h>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <QFileSystemWatcher>
#include <QAbstractItemModel>

using namespace std;


namespace CConf {

/*
class TreePath {
public:
    /// @param path A dot-separated key path
    TreePath(string path);

private:
    vector<string> _components;
};
*/



/**
 * @brief Error that occurs when loading a json file creates a type mismatch between nodes.

 * @details If one json file has a leaf type for a given path and another has an object (map/dict)
 * type, there is a type mismatch that can't be resolved.  Note that t's ok for leaves from different
 * files to be different types.
 */
class TypeMismatchError : public runtime_error {
public:
    TypeMismatchError(const char *what) : runtime_error(what) {}
};


class Node {
public:
    Node(Node *parent = nullptr) {
        _parent = parent;
    }

    ~Node() {
        if (_parent) delete _parent;
    }

    bool isMapNode() const {
        return _mappedSubnodes.size() > 0;
    }

    virtual bool isLeafNode() const = 0;

    // Node *&operator[](int idx) {
    //     if (isArrayNode()) {
    //         return _indexedSubnodes[idx];
    //     } else {
    //         throw invalid_argument("Attempt to index into non-array Node");
    //     }
    // }

    //  FIXME: only include in BranchNode?
    virtual Node *&operator[](const string &key) = 0;


    //  Methods for QAbstractItemModel
    ////////////////////////////////////////////////////////////////////////////////////

    QVariant data(int column) const {
        //  FIXME: implement
        return "Data";
    }

    int columnCount() const {
        return 1;   //  FIXME
    }

    int childCount() const {
        return max(_mappedSubnodes.size(), _indexedSubnodes.size());
    }

    Node *_childAtIndex(int index) {
        if (isMapNode()) {
            throw invalid_argument("Justin has yet to implement this");
            auto itr = _mappedSubnodes.begin();
            return itr->second;
        } else {
            throw invalid_argument("Attempt to get child of leaf Node");
            return nullptr;
        }
    }

    int row() {
        if (_parent) {
            return _parent->indexOfSubnode(this);
        } else {
            return 0;
        }
    }

    int indexOfSubnode(const Node *child) const {
        if (isMapNode()) {
            throw invalid_argument("Justin needs to implement this too");
        } else {
            throw invalid_argument("Attempt to get ask leaf node about its subnodes");
        }
    }

    Node *parent() {
        return _parent;
    }


protected:
    friend class Context;

    map<string, Node*> mappedSubnodes() { return _mappedSubnodes; }
    const map<string, Node*> mappedSubnodes() const { return _mappedSubnodes; }

    vector<Node*> indexedSubnodes() { return _indexedSubnodes; }
    const vector<Node*> indexedSubnodes() const { return _indexedSubnodes; }

private:
    map< vector<string>, Json::Value > _valuesByScope;

    //  if this is a parent node
    map<string, Node*> _mappedSubnodes;
    vector<Node*> _indexedSubnodes;
    Node *_parent;
};


////////////////////////////////////////////////////////////////////////////////


class BranchNode : public Node {
public:
    BranchNode(BranchNode *parent = nullptr) : Node(parent) {}

    bool isLeafNode() const {
        return false;
    }

    Node *&operator[](const string &key) {
        _subnodes[key];
    }

private:
    map<string, Node*> _subnodes;
};


////////////////////////////////////////////////////////////////////////////////


class ValueNode : public Node {
public:
    ValueNode(BranchNode *parent, const QVariant &value) : Node(parent) {}

    const bool isDefaultScope() const {
        return _scope.size() == 0;
    }

    const vector<string> &scope() const {
        return _scope;
    }

    const QVariant &value() const { return _value; }
    QVariant &value() { return _value; }

private:
    vector<string> _scope;
    string _filePath;
    QVariant _value;
};



class LeafNode : public Node {
public:
    LeafNode(BranchNode *parent = nullptr) : Node(parent) {}

    bool isLeafNode() const {
        return true;
    }


private:
    vector<ValueNode> _valueNodes;
};



////////////////////////////////////////////////////////////////////////////////


class Context : public QAbstractItemModel {
    Q_OBJECT

public:
    Context() {
        QObject::connect(&_fsWatcher, &QFileSystemWatcher::fileChanged, this, &Context::fileChanged);

        _rootNode = new BranchNode();
    }

    void fileChanged(const QString &filePath) {
        cout << "Context file changed: " << filePath.toStdString() << endl;
    }

    void addFile(const string &path) {
        if (containsFile(path)) {
            throw "this config file is already loaded";
        }

        Json::Value tree;
        readFile(path, tree);   //  note: throws an exception on failure

        try {
            merge(_rootNode, tree, path);
        } catch (TypeMismatchError e) {
            cerr << "Encountered a type mismatch when trying to load file: " << path << endl;
            cerr << "  Unloading all values from this file and rethrowing.  Correct and try again." << endl;
            throw e;
        }

        _configFiles.push_back(path);
        _fsWatcher.addPath(QString::fromStdString(path));
    }

    void readFile(const string &filePath, Json::Value jsonOut) {
        ifstream doc(filePath);
        Json::Reader reader;
        if (!reader.parse(doc, jsonOut)) {
            throw runtime_error("failed to parse json: " + reader.getFormattedErrorMessages());
        }
    }


    bool containsFile(const string &path) {
        return indexOfFile(path) != -1;
    }


    void removeFile(const string &filePath) {
        int idx = indexOfFile(filePath);
        if (idx == -1) {
            cerr << "Warning: Attempt to remove file from Context that's not in the context: " << filePath;
        } else {
            _configFiles.erase(_configFiles.begin() + idx);
            _fsWatcher.removePath(QString::fromStdString(filePath));
        }
    }


    /// returns -1 if the file isn't a part of this context
    int indexOfFile(const string &path) {
        auto itr = std::find(_configFiles.begin(), _configFiles.end(), path);
        return (itr == _configFiles.end()) ? -1 : itr - _configFiles.begin();
    }


    #pragma mark - QAbstractItemModel Stuff -

    //  see the article on Qt's website for more info on how to subclass QAbstractItemModel
    //  http://qt-project.org/doc/qt-4.8/itemviews-simpletreemodel.html


    QModelIndex index(int row, int column, const QModelIndex &parent) const {
        //  return invalid model index if the request was invalid
        if (!hasIndex(row, column, parent)) return QModelIndex();

        Node *parentNode = parent.isValid() ? static_cast<Node*>(parent.internalPointer()) : _rootNode;

        Node *childNode = parentNode->_childAtIndex(row);
        return childNode != nullptr ? createIndex(row, column, childNode) : QModelIndex();
    }

    QModelIndex parent(const QModelIndex &child) const {
        if (!child.isValid()) return QModelIndex();

        Node *childNode = static_cast<Node*>(child.internalPointer());
        Node *parentNode = childNode->parent();

        if (parentNode == _rootNode) return QModelIndex();

        return createIndex(parentNode->row(), 0, parentNode);
    }


    int rowCount(const QModelIndex &parent = QModelIndex()) const {
        const Node *parentNode;
        if (parent.column() > 0) return 0;

        if (!parent.isValid()) {
            parentNode = _rootNode;
        } else {
            parentNode = static_cast<const Node*>(parent.internalPointer());
        }

        return parentNode->childCount();
    }


    int columnCount(const QModelIndex &parent = QModelIndex()) const {
        if (parent.isValid()) {
            return static_cast<const Node*>(parent.internalPointer())->columnCount();
        } else {
            return _rootNode->columnCount();
        }
    }


    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
        if (!index.isValid()) return QVariant();

        if (role != Qt::DisplayRole) return QVariant();

        const Node *node = static_cast<const Node*>(index.internalPointer());
        return node->data(index.column());
    }


    Qt::ItemFlags flags(const QModelIndex &index) const {
        if (!index.isValid()) return 0;

        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;    //  FIXME: this is readonly - eventually we'll make it readwrite
    }


    QVariant headerData(int section, Qt::Orientation orientation, int role) const {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            return "CConf";
        }

        return QVariant();
    }




    //  Deep shit
    //////////////////////////////////////////////////////////////////////////////////////
protected:

    void _didInsert(const QModelIndex &index) {
        //  FIXME: emit, notify
    }

    void _didDelete(const QModelIndex &index) {
        //  FIXME: emit, notify
    }

    void _didChange(const QModelIndex &index) {
        //  FIXME: emit, notify
    }


    /**
     * recursive method to apply the new json value on top of the config (sub)tree
     * calls _didInsert, _didDelete, _didChange as it goes
     * respects the priorities already present and only replaces if the given json has higher prioity
     * then another leaf node with the same key path.
     * 
     * If a type mismatch is encountered, the TypeMismatchError is thrown, but the values are not
     * removed from the tree - that's the job of the caller.
     * 
     * @param node the root of the (sub)tree to merge the new values onto
     * @param json the json to merge onto the given tree
     * @param filePath the file path of the json - used to lookup the priority of @json as necessary
     */
    void merge(Node *node, const Json::Value &json, const string &filePath) {




        throw invalid_argument("TODO");
    }


    /**
     * @brief When we remove a file from the context, we need to remove the values from the tree
     * 
     * @param filePath the path of the file we're removing
     */
    void removeValuesFromFile(Node *node, const string &filePath) {
        throw invalid_argument("TODO");
    }


    /**
     * Find the relative priority of a file.  Used when merging to determine whether or not to overwrite a value.
     * @param filePath Where the file lives
     * @return the relative priority of the file.  Higher values indicate greater importance/priority
     */
    int priorityOfFile(const string &filePath) {
        return indexOfFile(filePath);
    }


    /**
     * @brief Extract the json for the given file so it can be written to disk
     * 
     * @details We don't store a Json::Value for trees in the context.  We load it from a file,
     * then store it in our custom tree structure.  To write it out to disk, we have have to
     * convert it from our internal representation back to json.
     * 
     * @param filePath The path of the file we're extracting for.
     * @return 
     */
    void extractJson(const string &filePath, Json::Value &json) {
        throw invalid_argument("TODO");
    }



private:
    //  higher index = higher precedence when cascading values. FIXME: is this true?
    vector<string> _configFiles;
    BranchNode *_rootNode;
    QFileSystemWatcher _fsWatcher;
};





template<typename T>
class ConfigValue {
public:
    ConfigValue(string keyPath, T defaultValue, string comment = "");
    ConfigValue(shared_ptr<Context> ctxt, string keyPath, T defaultValue, string comment = "");

    void setContext(Context *ctxt) {
        _context = ctxt;
    }

    Context context() {
        return _context;
    }

    const string &comment() const {
        return _comment;
    }


    // *
    //  * The file to write newly-inserted values to.
    //  * 
    //  * @param writeToFile the file path
    // void setWriteToFile(string writeToFile);


// public slots:
//     void valueChanged(const T &newValue);


private:
    string _keyPath;
    T _value;
    vector<string> _scope;
    string _comment;
    shared_ptr<Context> _context;
};





class ConfigDouble : public ConfigValue<double> {
public:
    ConfigDouble(string keyPath, double defaultValue = 0, string comment = "");
    operator double();
};


};  //  end namespace CConf