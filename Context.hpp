
#pragma once

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "Tree.hpp"
#include "Item.hpp"


namespace CConf {
	class Context;

	class KeyPathObserver {
	public:
		KeyPathObserver(Context &ctxt, KeyPath &keyPath) : _context(ctxt), _keyPath(keyPath);
		~KeyPathObserver();

		KeyPath& keyPath() {
			return _keyPath;
		}

		Context& context() {
			return _context;
		}


	protected:
		//	the context calls this method when the value the observer is observing has changed
		virtual void notify() {}

	private:
		Context &_context;
		KeyPath _keyPath;
	};


	//================================================================================


	class Context {
	public:
		Context(const char *filePath) {
			std::string fp = filePath;
			_loadManifest(fp);
		}

		Context(std::string &filePath) {
			_loadManifest(filePath);
		}

		Context() {}


		boost::shared_ptr<Tree> load(const char *filePath) {
			std::string fp = filePath;
			return this->load(fp);
		}

		//	loads a ConfigTree form the given @filePath and inserts it at index 0 (giving it the highest priority)
		boost::shared_ptr<Tree> load(std::string filePath) {
			boost::shared_ptr<Tree> tree(new Tree(filePath));
			this->insertTree(tree);
			return tree;
		}

		void insertTree(boost::shared_ptr<Tree> tree, int index = 0) {
			_configTrees.insert(_configTrees.begin() + index, tree);
			//	FIXME: trigger
		}

		void removeTree(boost::shared_ptr<Tree> tree) {
			auto itr = std::find(_configTrees.begin(), _configTrees.end(), tree);
			if ( itr != _configTrees.end() ) {
				this->removeTree(itr - _configTrees.begin());
			}
		}

		void removeTree(int index) {
			_configTrees.erase(_configTrees.begin() + index);
			//	FIXME: trigger
		}

		void clear() {
			_configTrees.clear();
			//	FIXME: trigger
		}

		void save() {
			for ( auto itr = _configTrees.begin(); itr != _configTrees.end(); itr++ ) {
				boost::shared_ptr<Tree> tree = *itr;
				tree->write();
			}
		}

		Tree &derivedTree() {
			return _derivedTree;
		}


	protected:
		friend class KeyPathObserver;

		/**
		<Searches through the Context and sets the @tree and @node of the record
		based on which Tree in the Context has highest precedence for the given key path>

		@return true if the record needed updating
		*/
		bool _updateRecord(_ObservationRecord &record) {

		}

		bool _hasObserversForKeyPath(KeyPath &keyPath) {
			return _observationRecords.find(keyPath) != _observationRecords.end();
		}

		void registerObserver(KeyPathObserver *observer) {
			KeyPath keyPath = observer->keyPath();

			//	create the record if necessary
			if ( !_hasObserversForKeyPath(keyPath) ) {
				_observationRecords[keyPath] = _ObservationRecord();
				_observationRecords[keyPath].keyPath = keyPath;
				_updateRecord(_observationRecords[keyPath]);
			}

			_ObservationRecord &record = _observationRecords[keyPath];


			//	assert on double-registration attempts
			assert(record.observers.find(observer) == record.observers.end());

			record.observers.push_back(observer);

			//	FIXME: notify?

			//	FIXME:
		}

		void unregisterObserver(KeyPathObserver *observer) {

		}


	protected:
		//	triggers change notifications on all of the observers for the given keyPath
		void _keyPathChanged(KeyPath &keyPath) {
			_ObservationRecord &record = _observationRecords[keyPath];
			record.observers
		}

		void _loadManifest(std::string &filePath) {
			std::ifstream is(filePath.c_str());
		 	json_spirit::Value manifest;
		 	json_spirit::read(is, manifest);

		 	assert(manifest.type() == json_spirit::array_type);

		 	json_spirit::Array loadList = manifest.get_array();
		 	for ( int i = 0; i < loadList.size(); i++ ) {
		 		std::string loadPath = loadList[i].get_str();
		 		this->load(loadPath);
		 		// cout << "Loaded: " << loadPath << endl;
		 	}

	 		//	FIXME: what if file doesn't exist?
	 		//	FIXME: what if manifest isn't an array?
		}


	private:
		class _ObservationRecord {
			KeyPath keyPath;

			Tree tree;	//	the tree that the value cascades from	//	FIXME: what if it's an array?
			Value &node;

			std::set< KeyPathObserver* > observers;	//	observers of this key path
		};

		std::map< KeyPath, _ObservationRecord > _observationRecords;	//	observation records

		//	trees at lower indices take precedence over trees at higher indices
		std::vector< boost::shared_ptr<Tree> > _configTrees;
		Tree _transientTree;
		Tree _derivedTree;
	};
}
