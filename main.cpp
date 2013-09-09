
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>
#include "CConf.hpp"


using namespace json_spirit;
using namespace std;


int main(int argc, char **argv) {
	CConf::Context ctxt("manifest.json");


	CConf::KeyPath kp("setting_a");
	cout << "Key Path: " << kp.description() << endl;


	CConf::ItemImpl<int> item(ctxt, kp, 0);
	int val = *item;
	cout << "value: " << val << endl;


	ctxt.save();

	return 0;
}
