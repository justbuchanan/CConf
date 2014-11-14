#pragma once


class ConfigTree {
public:
    ConfigTree(QFile file);
    ConfigTree(QString filePath);
    ConfigTree(QJsonDocument json);

protected:
    QJsonDocument _json;
    QFile _file;
};
