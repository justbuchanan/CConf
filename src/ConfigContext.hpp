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
#include <QFileSystemWatcher>
#include <QAbstractItemModel>

namespace CConf {

// TODO: description
const extern std::string CConfScopeKeyPrefix;

/// @brief Error that occurs when loading a json file creates a type mismatch
/// between nodes.

/// @details If one json file has a leaf type for a given path and another has
/// an object (map/dict) type, there is a type mismatch that can't be resolved.
/// Note that t's ok for leaves from different files to be different types.
class TypeMismatchError : public std::runtime_error {
 public:
  TypeMismatchError(const std::string &what) : std::runtime_error(what) {}
};

class Context;
class BranchNode;

class Node {
 public:
  Node(BranchNode *parent = nullptr, Context *context = nullptr);
  virtual ~Node();

  virtual bool isLeafNode() const = 0;

  virtual void removeValuesFromFile(const std::string &filePath) = 0;

  std::string keyPath() const;

  virtual QVariant data(int column) const = 0;
  virtual int childCount() const = 0;
  virtual int row();

  BranchNode *parent() { return _parent; }
  const BranchNode *parent() const { return _parent; }

  const Context *context() const { return _context; }

 protected:
  friend class BranchNode;
  friend class LeafNode;

  void setContext(Context *context) { _context = context; }

  Context *context() { return _context; }

 private:
  void _prependKeyPath(std::string *keyPathOut) const;

  Context *_context;
  BranchNode *_parent;
};

////////////////////////////////////////////////////////////////////////////////

class BranchNode : public Node {
 public:
  BranchNode(BranchNode *parent = nullptr, Context *context = nullptr)
      : Node(parent, context) {}

  bool isLeafNode() const { return false }
  int childCount() const;

  // int columnCount() const;
  QVariant data(int column) const;

  Node *operator[](const std::string &key);
  Node *_childAtIndex(int index);
  int indexOfSubnode(const Node *child) const;

  void removeValuesFromFile(const std::string &filePath);

  void getSubnodeKeys(std::set<std::string> *keysOut);

  std::string keyForSubnode(const Node *subnode) const;

 protected:
  friend class Context;

  void addSubnode(Node *node, const std::string &key);
  void removeSubnode(const std::string &key);

 protected:
  std::vector<std::string> _subnodeOrder;  //  alphabetically ordered keys
  // TODO: use unique_ptr to subnodes
  std::map<std::string, Node *> _subnodes;
};

////////////////////////////////////////////////////////////////////////////////

class ValueNode : public Node {
 public:
  ValueNode(const QVariant &value, const std::string &filePath,
            const std::vector<std::string> &scope = {})
      : _value(value), _filePath(filePath), _scope(scope) {}

  bool isDefaultScope() const { return _scope.size() == 0; }

  bool isLeafNode() const { return true; }

  void removeValuesFromFile(const)

      const std::vector<std::string> &scope() const {
    return _scope;
  }
  const std::string &filePath() const { return _filePath; }

  const QVariant &value() const { return _value; }

 private:
  QVariant _value;
  std::string _filePath;
  std::vector<std::string> _scope;
};

////////////////////////////////////////////////////////////////////////////////

class LeafNode : public Node {
 public:
  LeafNode(BranchNode *parent = nullptr, Context *context = nullptr)
      : Node(parent, context) {}

  bool isLeafNode() const { return false; }

  int childCount() const {
    return _values.size();  //  FIXME
  }

  // int columnCount() const;
  QVariant data(int column) const;

  void removeValuesFromFile(const std::string &filePath);

  void addValue(
      const QVariant &val, const std::string &filePath,
      const std::vector<std::string> &scope = std::vector<std::string>());

  /**
   * @brief Get the QVariant value of this leaf with the highest priority
   *
   * @details You can optionally specify the scope and/or filePath to refine the
   * query.
   * @param scope The scope you want to search in.  If no value is present for
   * this exact scope,
   * it searches subscopes.  Passing an empty scope vector searches the default
   * scope.
   * @param filepath The path of the file that the value should come from
   * @return A QVariant* containing the value.  If no value matches the query,
   * returns nullptr
   */
  const QVariant *getValue(
      const std::vector<std::string> &scope = std::vector<std::string>(),
      const std::string &filePath = "") const;

 private:
  std::vector<ValueNode> _values;
};

////////////////////////////////////////////////////////////////////////////////

class Context : public QAbstractItemModel {
  Q_OBJECT

 public:
  Context();

  void fileChanged(const QString &filePath);

  void addFile(const std::string &path);

  Json::Value readFile(const std::string &filePath);

  bool containsFile(const std::string &path);

  void removeFile(const std::string &filePath);

  /// returns -1 if the file isn't a part of this context
  int indexOfFile(const std::string &path) const;

  /**
   * Find the relative priority of a file.
   * @param filePath Where the file lives
   * @return the relative priority of the file.  Higher values indicate greater
   * importance/priority
   */
  int priorityOfFile(const std::string &filePath) const {
    return indexOfFile(filePath);
  }

  //  Methods for QAbstractModel
  //  see the article on Qt's website for more info on how to subclass
  //  QAbstractItemModel
  //  http://qt-project.org/doc/qt-4.8/itemviews-simpletreemodel.html
  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &child) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  /**
   * @brief Determine whether the key specifies a (sub)scope rather than a
   * regular keypath
   * @details Scopes paths are prefixed with '$$', so we check to see if @key
   * has this prefix.
   *
   * @param key the json key string
   * @return whether or not it's a scope specifier
   */
  static bool keyIsJsonScopeSpecifier(const std::string &key);

  static std::string extractKeyFromJsonScopeSpecifier(
      const std::string &scopeSpec);

 protected:
  /**
   * Anything that isn't a json 'object' type is stored in the tree in a leaf
   * node as a QVariant.
   * This method creates the corresponding QVariant from a json value.
   * See <json/value.h> for a list of available types
   */
  static QVariant variantValueFromJson(const Json::Value &json);

  /**
   * recursive method to apply the new json value on top of the config (sub)tree
   * calls _didInsert, _didDelete, _didChange as it goes
   * respects the priorities already present and only replaces if the given json
   * has higher prioity
   * then another leaf node with the same key path.
   *
   * If a type mismatch is encountered, the TypeMismatchError is thrown, but the
   * values are not
   * removed from the tree - that's the job of the caller.
   *
   * As it walks the tree, it removes any values from the @node and its subnodes
   * that came from
   * @filePath, but are not in @json.
   *
   * @param node the root of the (sub)tree to merge the new values onto
   * @param json the json to merge onto the given tree
   * @param filePath the file path of the json - used to lookup the priority of
   * @json as necessary
   */
  void mergeJson(Node *node, const Json::Value &json,
                 std::vector<std::string> &scope, const std::string &filePath,
                 std::set<std::string> &unhandledKeys,
                 bool removeValuesForUnhandledKeys = true);

  /**
   * @brief Extract the json for the given file so it can be written to disk
   *
   * @details We don't store a Json::Value for trees in the context.  We load it
   * from a file,
   * then store it in our custom tree structure.  To write it out to disk, we
   * have have to
   * convert it from our internal representation back to json.
   *
   * @param filePath The path of the file we're extracting for.
   * @return
   */
  Json::Value extractJson(const std::string &filePath) {
    throw std::invalid_argument("TODO");
  }

 private:
  //  higher index = higher precedence when cascading values. FIXME: is this
  //  true?
  std::vector<std::string> _configFiles;
  BranchNode *_rootNode;
  QFileSystemWatcher _fsWatcher;
};

template <typename T>
class ConfigValue {
 public:
  // ConfigValue(const std::string& keyPath, T defaultValue, const std::string&
  // comment = "");
  ConfigValue(std::shared_ptr<Context> ctxt, const std::string &keyPath,
              T defaultValue, const std::string &comment = "");

  void setContext(Context *ctxt) { _context = ctxt; }

  const Context *context() const { return _context; }

  const std::string &comment() const { return _comment; }

  // TODO: operator dereferene

  // TODO: signal for value change?

  // TODO: add scope

 private:
  std::string _keyPath;
  T _value;
  std::vector<std::string> _scope;
  std::string _comment;
  std::shared_ptr<Context> _context;
};

class ConfigDouble : public ConfigValue<double> {
 public:
  ConfigDouble(const std::string &keyPath, double defaultValue = 0,
               const std::string &comment = "");
  operator double();
};

class ConfigString : public ConfigValue<std::string> {
 public:
  ConfigString(const std::string &keyPath, const std::string &defaultValue = "",
               const std::string &comment = "");
  operator std::string();
};

// template<typename T>
// class ConfigVector : public ConfigValue<std::vector<T> > {
// public:
//     ConfigVector(const std::string& keyPath, const std::vector<T>&
//     defaultValue = {}, const std::string& comment = "");
// };

// class ConfigVectorDouble : public ConfigVector<double> {
// public:
//     ConfigVector(const std::string& keyPath, const std::vector<double>&
//     defaultValue = {}, const std::string& comment = "");
// };

};  //  end namespace CConf
