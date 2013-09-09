
#pragma once

#include <string>
#include "KeyPath.hpp"
#include "Context.hpp"
#include "KeyPathObserver.hpp"


namespace CConf {
	class Item : public KeyPathObserver {
	public:
		Item(Context &ctxt, KeyPath &keyPath) : KeyPathObserver(ctxt, keyPath) {}
		Item(Context &ctxt, std::string &keyPathStr) : KeyPathObserver(ctxt, keyPathStr) {}

		json_spirit::Value *treeNode() {
			lookupIfNeeded();
			return _value;
		}

	protected:
		void setNeedsLookup() {
			_needsLookup = true;
		}

		void lookupIfNeeded() {
			if ( _needsLookup ) lookup();
		}

		void lookup() {
			_value = Context::lookup(context(), keyPath(), &_tree);
			_needsLookup = false;
		}

	private:
		bool _needsLookup;
		json_spirit::Value *_value;
	};


	//================================================================================


	template<typename T>
	class ItemImpl : public Item {
	public:
		ItemImpl(Context &ctxt, KeyPath &keyPath, T &defaultValue = 0) : Item(ctxt, keyPath) {}
		ItemImpl(Context &ctxt, std::string &keyPathStr, T &defaultValue = 0) : Item(ctxt, KeyPath(keyPathStr)) {}

		T &value() {
			//	FIXME: look it up in the context

			// T t;
			// return t;
		}

		T &operator*() {
			return value();
		}

	private:
		T _defaultPrimitiveValue;

		json_spirit::Value *_value;
	};
}
