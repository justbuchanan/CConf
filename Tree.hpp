
#pragma once

#include <string>
#include <assert.h>
#include "json_spirit.h"


namespace CConf {
	class Context;


	class Tree {
	public:
		Tree(std::string filePath) : _value(json_spirit::Object()) {
			_filePath = filePath;

			//	read the contents of the file into _value
			std::ifstream ifs(_filePath.c_str());
			bool success = json_spirit::read(ifs, _value);

			//	FIXME: what if no success?
		}

		Tree(json_spirit::Value &val) {
			assert(val.type() == json_spirit::obj_type);	//	root object must be a dictionary
			_value = val;
		}

		Tree() {

		}


		void write(std::ofstream &ofs) {
			json_spirit::write_formatted(_value, ofs);	//	pretty print write
		}

		void write(std::string &filePath) {
			std::ofstream ofs;
			ofs.open(filePath.c_str(), std::ofstream::out);
			this->write(ofs);
		}

		void write() {
			this->write(_filePath);
		}

		const json_spirit::Value &value() const {
			return _value;
		}


	protected:
		friend class ConfigContext;

		// Value &lookup(ConfigKeyPath &kp) {
		// 	//	FIXME: ?
		// }


		json_spirit::Value &value() {
			return _value;
		}


	private:
		json_spirit::Value _value;
		std::string _filePath;
	};
}
