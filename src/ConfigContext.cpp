#include "ConfigContext.hpp"
#include <algorithm>
#include <cassert>

using namespace std;

namespace CConf {

const std::string CConfScopeKeyPrefix = "$$";

/**
 * Extracts all of the keys from the given map and puts them in the @keysOut set
 */
template <typename KeyType, typename ValueType>
void getMapKeys(const map<KeyType, ValueType> &theMap, set<KeyType> *keysOut) {
  for (auto itr : theMap) {
    keysOut->insert(itr.first);
  }
}

#pragma mark Node

Node::Node(Context *context, BranchNode *parent) {
  _parent = parent;
  _context = context;
}

Node::~Node() {}

int Node::row() {
  if (_parent) {
    return _parent->indexOfSubnode(this);
  } else {
    return 0;
  }
}

void Node::_prependKeyPath(string *keyPathOut) const {
  if (_parent) {
    keyPathOut->insert(0, ".");
    keyPathOut->insert(0, _parent->keyForSubnode(this));
    _parent->_prependKeyPath(keyPathOut);
  }
}

string Node::keyPath() const {
  string keyPath;
  _prependKeyPath(&keyPath);
  return keyPath;
}

#pragma mark BranchNode

Node *BranchNode::operator[](const string &key) { return _subnodes[key]; }

Node *BranchNode::_childAtIndex(int index) {
  return _subnodes[_subnodeOrder[index]];
}

int BranchNode::childCount() const { return _subnodes.size(); }

void BranchNode::addSubnode(Node *node, const string &key) {
  if (_subnodes[key] != nullptr)
    throw invalid_argument(
        "Attempt to add subnode for key that already exists: '" + key + "'");

  _subnodeOrder.push_back(key);
  std::sort(_subnodeOrder.begin(), _subnodeOrder.end());

  _subnodes[key] = node;

  node->setContext(context());
}

void BranchNode::removeSubnode(const string &key) {
  _subnodeOrder.erase(
      std::find(_subnodeOrder.begin(), _subnodeOrder.end(), key));

  Node *node = _subnodes[key];
  node->setContext(nullptr);
  _subnodes.erase(key);
  delete node;
}

void BranchNode::getSubnodeKeys(set<string> *keysOut) {
  getMapKeys<string, Node *>(_subnodes, keysOut);
}

string BranchNode::keyForSubnode(const Node *subnode) const {
  for (auto itr : _subnodes) {
    if (itr.second == subnode) {
      return itr.first;
    }
  }

  throw invalid_argument(
      "keyForSubnode() called for node that isn't a subnode");
}

int BranchNode::indexOfSubnode(const Node *child) const {
  return std::find(_subnodeOrder.begin(), _subnodeOrder.end(),
                   keyForSubnode(child)) -
         _subnodeOrder.begin();
}

void BranchNode::removeValuesFromFile(const string &filePath) {
  for (auto itr : _subnodes) {
    itr.second->removeValuesFromFile(filePath);
  }
}

// int BranchNode::columnCount() const {
//     return 1;
// }

QVariant BranchNode::data(int column) const {
  if (column == 0) {
    return QVariant(QString::fromStdString(
        (parent() != nullptr) ? parent()->keyForSubnode(this) : "CConf Root"));
  } else {
    return "";
  }
}

// #pragma mark LeafNode

// void LeafNode::removeValuesFromFile(const string &filePath) {
//   for (int i = 0; i < _values.size();) {
//     if (_values[i].filePath() == filePath)
//       _values.erase(_values.begin() + i);
//     else
//       i++;
//   }
// }

// void LeafNode::addValue(const QVariant &val, const string &filePath,
//                         const vector<string> &scope) {
//   cout << "Added value to keypath '" << keyPath() << "' with scope ";
//   for (string key : scope) cout << key << ".";
//   cout << "'; filePath '" << filePath << "'" << endl;
//   _values.push_back(ValueNode(val, filePath, scope));
// }

// // int LeafNode::columnCount() const {
// //     return 2;
// // }

// QVariant LeafNode::data(int column) const {
//   if (column == 0) {
//     return QVariant(QString::fromStdString(
//         (parent() != nullptr) ? parent()->keyForSubnode(this) : "CConf Root"));
//   } else if (column == 1) {
//     const QVariant *val = getValue();
//     return (val != nullptr) ? *val : QVariant(QString("<null>"));
//   } else {
//     throw invalid_argument("Invalid column index for leaf node");
//   }
// }

// const QVariant *LeafNode::getValue(const vector<string> &scope,
//                                    const string &filePath) const {
//   //  TODO: optimize this method - probably by maintaining additional data
//   //  structures to make this search faster

//   bool fileSpecified = filePath.length() > 0;

//   vector<ValueNode *> matchingEntries;

//   for (int maxScopeIndex = scope.size() - 1; maxScopeIndex >= -1;
//        maxScopeIndex--) {
//     for (auto entryItr : _values) {
//       // cout << "Examining entry with fp = " << entryItr.filePath() << "; value
//       // = " << entryItr.value().toString().toStdString() << endl;
//       if (!fileSpecified || entryItr.filePath() == filePath) {
//         if (entryItr.scope().size() == maxScopeIndex + 1) {
//           bool found = true;
//           for (int i = 0; i <= maxScopeIndex; i++) {
//             if (entryItr.scope()[i] != scope[i]) {
//               found = false;
//               break;
//             }
//           }
//           if (found) matchingEntries.push_back(&entryItr);
//         }
//       }
//     }
//     if (matchingEntries.size() > 0 || (scope.size() == 0 && fileSpecified))
//       break;
//   }

//   cout << "at getPath() for '" << keyPath()
//        << "', value count = " << _values.size() << endl;

//   if (matchingEntries.size() > 0) {
//     //  sort by file priority
//     std::sort(matchingEntries.begin(), matchingEntries.end(),
//               [&](const ValueNode *a, const ValueNode *b) -> bool {
//                 return context()->priorityOfFile(a->filePath()) >
//                        context()->priorityOfFile(b->filePath());
//               });

//     //  value with highest priority
//     return &(matchingEntries.back()->value());

//   } else {
//     return nullptr;
//   }
// }

#pragma mark Context

/**
 * Remove keys that are merged from the @unhandledKeys set
 */
void Context::mergeJson(Node *node, const Json::Value &json,
                        vector<string> &scope, const string &filePath,
                        set<string> &unhandledKeys,
                        bool removeValuesForUnhandledKeys) {
  assert(node != nullptr);

  if (node->isLeafNode() == (json.type() == Json::objectValue)) {
    throw TypeMismatchError("Attempt to merge a tree and a leaf at key path '" +
                            node->keyPath() + "'.");
  }

  if (node->isLeafNode()) {
    if (removeValuesForUnhandledKeys) node->removeValuesFromFile(filePath);

    ((ValueNode*)node)->addValue(variantValueFromJson(json), filePath, scope);
  } else {
    BranchNode *parentNode = (BranchNode *)node;

    for (Json::ValueIterator itr = json.begin(); itr != json.end(); itr++) {
      string jsonKey = itr.key().asString();

      //  handle scopes
      if (keyIsJsonScopeSpecifier(jsonKey)) {
        scope.push_back(extractKeyFromJsonScopeSpecifier(jsonKey));
        mergeJson(node, json[jsonKey], scope, filePath, unhandledKeys, true);
        scope.pop_back();
      } else {
        Node *childNode = (*parentNode)[jsonKey];
        if (!childNode) {
          if (itr->type() == Json::objectValue) {
            childNode = new BranchNode(this, parentNode);
          } else {
            childNode = new ValueNode(this, parentNode);
          }
          parentNode->addSubnode(childNode, jsonKey);

          cout << "Appended new node for key: " << jsonKey << endl;
        }

        unhandledKeys.erase(jsonKey);

        set<string> childUnhandledKeys;
        if (!childNode->isLeafNode())
          ((BranchNode *)childNode)->getSubnodeKeys(&childUnhandledKeys);
        mergeJson(childNode, *itr, scope, filePath, childUnhandledKeys, true);
      }
    }

    //  other keys under this branch that this json didn't have anything for
    if (removeValuesForUnhandledKeys) {
      for (auto key : unhandledKeys) {
        (*parentNode)[key]->removeValuesFromFile(filePath);
      }
    }
  }
}

QVariant Context::variantValueFromJson(const Json::Value &json) {
  switch (json.type()) {
    case Json::nullValue:
      return QVariant();
    case Json::intValue:
      return QVariant(json.asInt());
    case Json::uintValue:
      return QVariant(json.asUInt());
    case Json::realValue:
      return QVariant(json.asDouble());
    case Json::stringValue:
      return QVariant(QString::fromStdString(json.asString()));
    case Json::booleanValue:
      return QVariant(json.asBool());
    case Json::arrayValue: {
      QList<QVariant> lst;
      for (auto itr : json) {
        lst.append(variantValueFromJson(itr));
      }
      return QVariant(lst);
    }
    default:
      throw invalid_argument(
          "Invalid json value passed to variantValueFromJson()");  //  TODO:
                                                                   //  more info
      return QVariant();
  }
}

bool Context::keyIsJsonScopeSpecifier(const string &key) {
  if (key.length() <= CConfScopeKeyPrefix.length()) return false;

  for (int i = 0; i < CConfScopeKeyPrefix.length(); ++i) {
    if (key[i] != CConfScopeKeyPrefix[i]) return false;
  }
  return true;
}

string Context::extractKeyFromJsonScopeSpecifier(const string &scopeSpec) {
  return scopeSpec.substr(CConfScopeKeyPrefix.length());
}

Context::Context() {
  QObject::connect(&_fsWatcher, &QFileSystemWatcher::fileChanged, this,
                   &Context::fileChanged);

  _rootNode = new BranchNode(this);
}

void Context::fileChanged(const QString &filePath) {
  cout << "Context file changed: " << filePath.toStdString() << endl;
}

void Context::addFile(const string &path) {
  if (containsFile(path)) {
    throw invalid_argument(
        "The given file is already present in the context: '" + path + "'");
  }

  //  note: throws an exception on failure
  Json::Value tree = readFile(path);

  try {
    vector<string> scope;
    set<string> rootKeys;
    _rootNode->getSubnodeKeys(&rootKeys);
    mergeJson(_rootNode, tree, scope, path, rootKeys, true);
  } catch (TypeMismatchError e) {
    cerr << "Encountered a type mismatch when trying to load file: " << path
         << endl;
    cerr << "  Unloading all values from this file and rethrowing.  Correct "
            "and try again." << endl;
    // TODO: unload values from the failed file
    throw e;
  }

  _configFiles.push_back(path);
  _fsWatcher.addPath(QString::fromStdString(path));
}

Json::Value Context::readFile(const string &filePath) {
  ifstream doc(filePath);
  Json::Reader reader;
  Json::Value json;
  if (!reader.parse(doc, json)) {
    throw runtime_error("failed to parse json: " +
                        reader.getFormattedErrorMessages());
  }
  return json;
}

bool Context::containsFile(const string &path) {
  return indexOfFile(path) != -1;
}

void Context::removeFile(const string &filePath) {
  int idx = indexOfFile(filePath);
  if (idx == -1) {
    cerr << "Warning: Attempt to remove file from Context that's not in the "
            "context: " << filePath << endl;
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

QModelIndex Context::index(int row, int column,
                           const QModelIndex &parent) const {
  //  return invalid model index if the request was invalid
  if (!hasIndex(row, column, parent)) return QModelIndex();

  BranchNode *parentNode =
      parent.isValid() ? static_cast<BranchNode *>(parent.internalPointer())
                       : _rootNode;

#warning This doesn't handle LeafNodes correctly - we tell Qt that LeafNode has subnodes, then here we assume each Node pointed to is a BranchNode...

  Node *childNode = parentNode->_childAtIndex(row);
  return childNode != nullptr ? createIndex(row, column, childNode)
                              : QModelIndex();
}

QModelIndex Context::parent(const QModelIndex &child) const {
  if (!child.isValid()) return QModelIndex();

  Node *childNode = static_cast<Node *>(child.internalPointer());
  BranchNode *parentNode = childNode->parent();

  if (parentNode == _rootNode) return QModelIndex();

  return createIndex(parentNode->row(), 0, parentNode);
}

int Context::rowCount(const QModelIndex &parent) const {
  if (parent.column() > 0) return 0;

  const Node *node;
  if (!parent.isValid()) {
    node = _rootNode;
  } else {
    node = static_cast<const Node *>(parent.internalPointer());
  }

  return node->childCount();
}

int Context::columnCount(const QModelIndex &parent) const { return 2; }

QVariant Context::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  if (role != Qt::DisplayRole) return QVariant();

  const Node *node = static_cast<const Node *>(index.internalPointer());
  return node->data(index.column());
}

Qt::ItemFlags Context::flags(const QModelIndex &index) const {
  if (!index.isValid()) return 0;

  //  FIXME: this is readonly - eventually we'll make it readwrite
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant Context::headerData(int section, Qt::Orientation orientation,
                             int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    return section == 0 ? "Key Path" : "Value";
  }

  return QVariant();
}

};  //   end namespace CConf
