
#include "KeyPath.hpp"


CConf::KeyPath::KeyPath(const char *str) {
	std::string s = str;
	_loadFromString(s);
}

CConf::KeyPath::KeyPath(std::string &str) {
	_loadFromString(str);
}

bool CConf::KeyPath::operator==(KeyPath &other) {
	return _components == other._components;
}

std::string CConf::KeyPath::description() {
	std::stringstream ss;
	ss << "<";
	for ( int i = 0; i < _components.size(); i++ ) {
		if ( i != 0 ) ss << ".";
		ss << _components[i];
	}
	ss << ">";

	return ss.str();
}

std::string &CConf::KeyPath::operator[](unsigned int i) {
	return _components[i];
}

void CConf::KeyPath::_loadFromString(std::string &str) {
	_components.clear();
	boost::char_separator<char> sep(".");
	boost::tokenizer< boost::char_separator<char> > tokens(str, sep);
	BOOST_FOREACH(const std::string& t, tokens) {
		_components.push_back(t);
	}
}

CConf::KeyPath::




