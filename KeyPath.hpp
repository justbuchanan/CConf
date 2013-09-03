
#pragma once

#include <string>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>


namespace CConf {
	class KeyPath {
	public:
		KeyPath(const char *str) {
			std::string s = str;
			_loadFromString(s);
		}

		KeyPath(std::string &str) {
			_loadFromString(str);
		}

		KeyPath(KeyPath &other) : _components(other._components) {}

		bool operator==(KeyPath &other) {
			//	FIXME

			return true;
		}

		std::string description() {
			std::stringstream ss;
			ss << "<";
			for ( int i = 0; i < _components.size(); i++ ) {
				if ( i != 0 ) ss << ".";
				ss << _components[i];
			}
			ss << ">";

			return ss.str();
		}


	protected:
		void _loadFromString(std::string &str) {
			_components.clear();
			boost::char_separator<char> sep(".");
			boost::tokenizer< boost::char_separator<char> > tokens(str, sep);
			BOOST_FOREACH(const std::string& t, tokens) {
				_components.push_back(t);
			}
		}


	private:
		std::vector< std::string > _components;
	};
}
