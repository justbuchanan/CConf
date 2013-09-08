
#pragma once

#include <string>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>


namespace CConf {
	class KeyPath {
	public:
		KeyPath(const char *str);
		KeyPath(std::string &str);
		KeyPath(KeyPath &other) : _components(other._components) {}

		bool operator==(KeyPath &other);
		std::string &operator[](unsigned int i);
		std::string description();


	protected:
		void _loadFromString(std::string &str);


	private:
		std::vector< std::string > _components;
	};
}
