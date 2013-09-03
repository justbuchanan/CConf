
#include "Context.hpp"


CConf::KeyPathObserver::KeyPathObserver(CConf::Context &ctxt, CConf::KeyPath &keyPath) : _context(ctxt), _keyPath(keyPath) {
	_context.registerObserver(this);
}

CConf::KeyPathObserver::~KeyPathObserver() {
	_context.unregisterObserver(this);
}
