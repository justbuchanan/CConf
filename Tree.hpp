
#pragma once

#include <string>
#include <assert.h>
#include <exception>
#include "json_spirit.h"
#include "KeyPath.hpp"


namespace CConf {
	class Context;

	class Tree {
	public:
		Tree(std::string &filePath) : _value(json_spirit::Object()) {
			_filePath = filePath;

			//	read the contents of the file into _value
			std::ifstream ifs(_filePath.c_str());
			bool success = json_spirit::read(ifs, _value);

			if ( !success ) {
				throw std::exception();	//("Failed to open file");
			}
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

		void setFiePath(std::string &fp) {
			_filePath = fp;
		}

		static json_spirit::Value &lookup(json_spirit::Value &value, KeyPath &kp, int offset = 0) {
			if ( offset == kp.length() ) {
				return value;
			} else if ( offset >= 0 && offset < kp.length() ) {
				if ( value.type() != json_spirit::obj_type ) throw std::exception();	//	Value not an object - cant traverse KeyPath
				std::string &key = kp[offset];
				json_spirit::Object obj = value.get_obj();
				return lookup(obj[key], offset - 1);
			} else {
				throw std::exception();	//	("Invalid offset for KeyPath");
			}
		}

	protected:
		json_spirit::Value &value() {
			return _value;
		}

	private:
		json_spirit::Value _value;
		std::string _filePath;
	};
}
