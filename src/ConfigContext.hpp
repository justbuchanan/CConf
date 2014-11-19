#pragma once

#include <json/json.h>
#include <vector>
#include <set>
#include <map>
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


/**
 * @brief Error that occurs when loading a json file creates a type mismatch between nodes.

 * @details If one json file has a leaf type for a given path and another has an object (map/dict)
 * type, there is a type mismatch that can't be resolved.  Note that t's ok for leaves from different
 * files to be different types.
 */
class TypeMismatchError : public runtime_error {
public:
    TypeMismatchError(const string &what) : runtime_error(what) {}
};



class Context;
class BranchNode;


class Node {
public:
    Node(BranchNode *parent = nullptr);
    ~Node();

    virtual bool isLeafNode() const = 0;


    void _prependKeyPath(string &keyPathOut) const;

    string keyPath() const {
        string keyPath;
        _prependKeyPath(keyPath);
        return keyPath;
    }


    QVariant data(int column) const {
        //  FIXME: implement
        return "Data";
    }

    int columnCount() const {
        return 1;   //  FIXME
    }

    virtual int childCount() const = 0;

    int row();

    BranchNode *parent() { return _parent; }
    const BranchNode *parent() const { return _parent; }


private:
    BranchNode *_parent;
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

    Node *_childAtIndex(int index) {
        throw invalid_argument("Justin has yet to implement this");
        return nullptr;
    }

    int childCount() const {
        return _subnodes.size();
    }

    int indexOfSubnode(const Node *child) const {
        throw invalid_argument("TODO");
        return -1;
    }


protected:
    friend class Context;
    friend class Node;

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


////////////////////////////////////////////////////////////////////////////////


class LeafNode : public Node {
public:
    LeafNode(BranchNode *parent = nullptr) : Node(parent) {}

    bool isLeafNode() const {
        return true;
    }

    int childCount() const {
        return 0;
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
            throw invalid_argument("The given file is already present in the context" + path);
        }

        Json::Value tree;
        readFile(path, tree);   //  note: throws an exception on failure

        try {
            vector<string>scope;
            mergeJson(_rootNode, tree, scope, path);
        } catch (TypeMismatchError e) {
            cerr << "Encountered a type mismatch when trying to load file: " << path << endl;
            cerr << "  Unloading all values from this file and rethrowing.  Correct and try again." << endl;
            throw e;
        }

        _configFiles.push_back(path);
        _fsWatcher.addPath(QString::fromStdString(path));
    }

    void readFile(const string &filePath, Json::Value &jsonOut) {
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

        BranchNode *parentNode = parent.isValid() ? static_cast<BranchNode*>(parent.internalPointer()) : _rootNode;

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
     * @brief Determine whether the key specifies a (sub)scope rather than a regular keypath
     * @details Scopes paths are prefixed with '$$', so we check to see if @key has this prefix.
     * 
     * @param key the json key string
     * @return whether or not it's a scope specifier
     */
    bool keyIsJsonScopeSpecifier(const string &key) {
        return key.length() >= 3 && key[0] == '$' && key[1] == '$';
    }


    /**
     * Extracts all of the keys from the given map and puts them in the @keysOut set
     */
    template<typename KeyType, typename ValueType>
    void getMapKeys(const map<KeyType, ValueType> &theMap, set<KeyType> &keysOut) {
        for (auto itr : theMap) {
            keysOut.insert(itr.first);
        }
    }


    //  TODO: replace std::find() calls with container.find()

    /**
     * Anything that isn't a json 'object' type is stored in the tree in a leaf node as a QVariant.
     * This method creates the corresponding QVariant from a json value.
     * See <json/value.h> for a list of available types
     */
    QVariant variantValueFromJson(const Json::Value &json) {
        switch (json.type()) {
            case Json::nullValue: return QVariant();
            case Json::intValue: return QVariant(json.asInt());
            case Json::uintValue: return QVariant(json.asUInt());
            case Json::realValue: return QVariant(json.asDouble());
            case Json::stringValue: return QVariant(QString::fromStdString(json.asString()));
            case Json::booleanValue: return QVariant(json.asBool());
            case Json::arrayValue:
            {
                QList<QVariant> lst;
                for (auto itr : json) {
                    lst.append(variantValueFromJson(itr));
                }
                return QVariant(lst);
            }
            default:
                throw invalid_argument("Invalid json value passed to variantValueFromJson()");  //  TODO: more info
                return QVariant();
        }
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
     * As it walks the tree, it removes any values from the @node and its subnodes that came from
     * @filePath, but are not in @json.
     * 
     * @param node the root of the (sub)tree to merge the new values onto
     * @param json the json to merge onto the given tree
     * @param filePath the file path of the json - used to lookup the priority of @json as necessary
     */
    void mergeJson(Node *node, const Json::Value &json, vector<string> &scope, const string &filePath) {
        if (node->isLeafNode() == (json.type() == Json::objectValue)) {
            throw TypeMismatchError("Attempt to merge a tree and a leaf at key path '" + node->keyPath() + "'.");
        }


        if (node->isLeafNode()) {
            throw invalid_argument("TODO");
            
        } else {
            BranchNode *parentNode = (BranchNode *)node;

            set<string> existingKeys;
            getMapKeys<string, Node*>(parentNode->_subnodes, existingKeys);

            for (Json::ValueIterator itr = json.begin(); itr != json.end(); itr++) {
                string jsonKey = itr.key().asString();

                if (existingKeys.find(jsonKey) != existingKeys.end()) {
                    existingKeys.erase(jsonKey);
                } else {
                    Node *newChildNode;
                    if (itr->type() == Json::objectValue) {
                        newChildNode = new BranchNode(parentNode);
                    } else {
                        newChildNode = new LeafNode(parentNode);
                    }
                    parentNode->_subnodes[jsonKey] = newChildNode;

                    cout << "Appended new node for key: " << jsonKey << endl;
                }

                mergeJson(parentNode->_subnodes[jsonKey], *itr, scope, filePath);
            }

            //  other keys under this branch that this json didn't have anything for
            for (auto key : existingKeys) {
                removeValuesFromFile(parentNode->_subnodes[key], filePath);
            }
        }
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