#include "ConfigContext.hpp"


CConf::Node::Node(BranchNode *parent) {
    _parent = parent;
}

CConf::Node::~Node() {
    delete _parent;
}

int CConf::Node::row() {
    if (_parent) {
        return _parent->indexOfSubnode(this);
    } else {
        return 0;
    }
}

void CConf::Node::_prependKeyPath(string &keyPathOut) const {
    if (_parent) {
        keyPathOut.insert(0, ".");
        for (auto itr : _parent->_subnodes) {
            if (itr.second == this) {
                keyPathOut.insert(0, itr.first);
                break;
            }
        }
        _parent->_prependKeyPath(keyPathOut);
    }
}
