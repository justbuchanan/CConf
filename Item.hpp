
#pragma once

#include <string>
#include "KeyPath.hpp"
#include "Context.hpp"


namespace CConf {
	class Item : public KeyPathObserver {
	public:
		Item(Context &ctxt, KeyPath &keyPath) : KeyPathObserver(ctxt, keyPath) {}
		Item(Context &ctxt, std::string &keyPathStr) : KeyPathObserver(ctxt, keyPathStr) {}

		json_spirit::Value &treeNode() {
			//	FIXME: lookup
		}
	};


	//================================================================================


	template<typename T>
	class ItemImpl : public Item {
	public:
		ItemImpl(Context &ctxt, KeyPath &keyPath, T &defaultValue = 0) : Item(ctxt, keyPath) {}
		ItemImpl(Context &ctxt, std::string &keyPathStr, T &defaultValue = 0) : Item(ctxt, keyPathStr) {}

		T &value() {
			//	FIXME: look it up in the context

			// T t;
			// return t;
		}

		T &operator*() {
			return value();
		}

	private:
		T _defaultValue;
	};
}
