#pragma once

#include <vector>

using namespace std;


class ConfigContext {
public:
    ConfigContext(QString...);

    void saveAll();


    void insertTree(shared_ptr<ConfigTree> tree, int index = 0);
    void removeTree(int index);

private:
    //  higher index = higher precedence when cascading values
    vector<shared_ptr<ConfigTree>> _configTrees;
    QJsonDocument _result;
};
