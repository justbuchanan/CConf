#include "ConfigContext.hpp"
#include <algorithm>


namespace CConf {


/**
 * Extracts all of the keys from the given map and puts them in the @keysOut set
 */
template<typename KeyType, typename ValueType>
void getMapKeys(const map<KeyType, ValueType> &theMap, set<KeyType> &keysOut) {
    for (auto itr : theMap) {
        keysOut.insert(itr.first);
    }
}


#pragma mark Node

Node::Node(BranchNode *parent, Context *context) {
    _parent = parent;
    _context = context;
}

Node::~Node() {
    delete _parent;
}

int Node::row() {
    if (_parent) {
        return _parent->indexOfSubnode(this);
    } else {
        return 0;
    }
}

void Node::_prependKeyPath(string &keyPathOut) const {
    if (_parent) {
        keyPathOut.insert(0, ".");
        keyPathOut.insert(0, _parent->keyForSubnode(this));
        _parent->_prependKeyPath(keyPathOut);
    }
}

string Node::keyPath() const {
    string keyPath;
    _prependKeyPath(keyPath);
    return keyPath;
}



#pragma mark BranchNode


bool BranchNode::isLeafNode() const {
    return false;
}

Node *BranchNode::operator[](const string &key) {
    return _subnodes[key];
}

Node *BranchNode::_childAtIndex(int index) {
    return _subnodes[ _subnodeOrder[index] ];
}

int BranchNode::childCount() const {
    return _subnodes.size();
}

void BranchNode::addSubnode(Node *node, const string &key) {
    if (_subnodes[key] != nullptr) throw invalid_argument("Attempt to add subnode for key that already exists: '" + key + "'");

    _subnodeOrder.push_back(key);
    std::sort(_subnodeOrder.begin(), _subnodeOrder.end());

    _subnodes[key] = node;

    node->setContext(context());
}

void BranchNode::removeSubnode(const string &key) {
    _subnodeOrder.erase(std::find(_subnodeOrder.begin(), _subnodeOrder.end(), key));

    Node *node = _subnodes[key];
    node->setContext(nullptr);
    _subnodes.erase(key);
    delete node;
}

void BranchNode::getSubnodeKeys(set<string> keysOut) {
    getMapKeys<string, Node*>(_subnodes, keysOut);
}

string BranchNode::keyForSubnode(const Node *subnode) const {
    for (auto itr : _subnodes) {
        if (itr.second == subnode) {
            return itr.first;
        }
    }

    throw invalid_argument("keyForSubnode() called for node that isn't a subnode");
}

int BranchNode::indexOfSubnode(const Node *child) const {
    return std::find(_subnodeOrder.begin(), _subnodeOrder.end(), keyForSubnode(child)) - _subnodeOrder.begin();
}

void BranchNode::removeValuesFromFile(const string &filePath) {
    for (auto itr : _subnodes) {
        itr.second->removeValuesFromFile(filePath);
    }
}

int BranchNode::columnCount() const {
    return 1;
}

QVariant BranchNode::data(int column) const {
    if (column == 0) {
        return QVariant(QString::fromStdString((parent() != nullptr) ? parent()->keyForSubnode(this) : "CConf Root"));
    } else {
        return "";
    }
}



#pragma mark LeafNode

void LeafNode::removeValuesFromFile(const string &filePath) {
    for (int i = 0; i < _values.size();) {
        if (_values[i].filePath() == filePath) _values.erase(_values.begin() + i);
        else i++;
    }
}

void LeafNode::addValue(const QVariant &val, const string &filePath, const vector<string> &scope) {
    _values.push_back(ValueEntry(val, filePath, scope));
}

int LeafNode::columnCount() const {
    return 2;
}

QVariant LeafNode::data(int column) const {
    if (column == 0) {
        return QVariant(QString::fromStdString((parent() != nullptr) ? parent()->keyForSubnode(this) : "CConf Root"));
    } else if (column == 1) {
        const QVariant *val = getValue();
        return (val != nullptr) ? *val : QVariant(QString("<null>"));
    } else {
        throw invalid_argument("Invalid column index for leaf node");
    }
}

const QVariant *LeafNode::getValue(const vector<string> &scope, const string &filePath) const {
    //  TODO: optimize this method - probably by maintaining additional data structures to make this search faster

    bool fileSpecified = filePath.length() > 0;

    vector<ValueEntry *> matchingEntries;

    for (int maxScopeIndex = scope.size() - 1; maxScopeIndex >= -1; maxScopeIndex--) {
        for (auto entryItr : _values) {
            // cout << "Examining entry with fp = " << entryItr.filePath() << "; value = " << entryItr.value().toString().toStdString() << endl;
            if (!fileSpecified || entryItr.filePath() == filePath) {
                if (entryItr.scope().size() == maxScopeIndex + 1) {
                    bool found = true;
                    for (int i = 0; i <=  maxScopeIndex; i++) {
                        if (entryItr.scope()[i] != scope[i]) {
                            found = false;
                            break;
                        }
                    }
                    if (found) matchingEntries.push_back(&entryItr);
                }
            }
        }
        if (matchingEntries.size() > 0 || (scope.size() == 0 && fileSpecified)) break;
    }

    if (matchingEntries.size() > 0) {
        //  sort by file priority
        std::sort(matchingEntries.begin(), matchingEntries.end(), [&](const ValueEntry *a, const ValueEntry *b) -> bool {
            return context()->priorityOfFile(a->filePath()) > context()->priorityOfFile(b->filePath());
        });

        //  value with highest priority
        return &(matchingEntries.back()->value());

    } else {
        return nullptr;
    }
}



#pragma mark Context

void Context::mergeJson(Node *node, const Json::Value &json, vector<string> &scope, const string &filePath) {
    if (node->isLeafNode() == (json.type() == Json::objectValue)) {
        throw TypeMismatchError("Attempt to merge a tree and a leaf at key path '" + node->keyPath() + "'.");
    }


    if (node->isLeafNode()) {
        node->removeValuesFromFile(filePath);
        
        ((LeafNode *)node)->addValue(variantValueFromJson(json), filePath, scope);
    } else {
        BranchNode *parentNode = (BranchNode *)node;

        set<string> existingKeys;
        parentNode->getSubnodeKeys(existingKeys);

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
                parentNode->addSubnode(newChildNode, jsonKey);

                cout << "Appended new node for key: " << jsonKey << endl;
            }

            mergeJson((*parentNode)[jsonKey], *itr, scope, filePath);
        }

        //  other keys under this branch that this json didn't have anything for
        for (auto key : existingKeys) {
            (*parentNode)[key]->removeValuesFromFile(filePath);
        }
    }
}

QVariant Context::variantValueFromJson(const Json::Value &json) {
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


bool Context::keyIsJsonScopeSpecifier(const string &key) {
    return key.length() >= 3 && key[0] == '$' && key[1] == '$';
}



Context::Context() {
    QObject::connect(&_fsWatcher, &QFileSystemWatcher::fileChanged, this, &Context::fileChanged);

    _rootNode = new BranchNode(nullptr, this);
}

void Context::fileChanged(const QString &filePath) {
    cout << "Context file changed: " << filePath.toStdString() << endl;
}

void Context::addFile(const string &path) {
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

void Context::readFile(const string &filePath, Json::Value &jsonOut) {
    ifstream doc(filePath);
    Json::Reader reader;
    if (!reader.parse(doc, jsonOut)) {
        throw runtime_error("failed to parse json: " + reader.getFormattedErrorMessages());
    }
}


bool Context::containsFile(const string &path) {
    return indexOfFile(path) != -1;
}


void Context::removeFile(const string &filePath) {
    int idx = indexOfFile(filePath);
    if (idx == -1) {
        cerr << "Warning: Attempt to remove file from Context that's not in the context: " << filePath;
    } else {
        _configFiles.erase(_configFiles.begin() + idx);
        _fsWatcher.removePath(QString::fromStdString(filePath));
    }
}

int Context::indexOfFile(const string &filePath) const {
    auto itr = std::find(_configFiles.begin(), _configFiles.end(), filePath);
    return (itr == _configFiles.end()) ? -1 : itr - _configFiles.begin();
}



#pragma mark Context - Item Model

QModelIndex Context::index(int row, int column, const QModelIndex &parent) const {
    //  return invalid model index if the request was invalid
    if (!hasIndex(row, column, parent)) return QModelIndex();

    BranchNode *parentNode = parent.isValid() ? static_cast<BranchNode*>(parent.internalPointer()) : _rootNode;

    Node *childNode = parentNode->_childAtIndex(row);
    return childNode != nullptr ? createIndex(row, column, childNode) : QModelIndex();
}

QModelIndex Context::parent(const QModelIndex &child) const {
    if (!child.isValid()) return QModelIndex();

    Node *childNode = static_cast<Node*>(child.internalPointer());
    BranchNode *parentNode = childNode->parent();

    if (parentNode == _rootNode) return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}


int Context::rowCount(const QModelIndex &parent) const {
    const Node *parentNode;
    if (parent.column() > 0) return 0;

    if (!parent.isValid()) {
        parentNode = _rootNode;
    } else {
        parentNode = static_cast<const Node*>(parent.internalPointer());
    }

    return parentNode->childCount();
}


int Context::columnCount(const QModelIndex &parent) const {
    return 2;

    // if (parent.isValid()) {
    //     return static_cast<const Node*>(parent.internalPointer())->columnCount();
    // } else {
    //     return _rootNode->columnCount();
    // }
}


QVariant Context::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role != Qt::DisplayRole) return QVariant();

    const Node *node = static_cast<const Node*>(index.internalPointer());
    return node->data(index.column());
}


Qt::ItemFlags Context::flags(const QModelIndex &index) const {
    if (!index.isValid()) return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;    //  FIXME: this is readonly - eventually we'll make it readwrite
}


QVariant Context::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "CConf";
    }

    return QVariant();
}


}; //   end namespace CConf
