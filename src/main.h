#ifndef MAIN_H
#define MAIN_H

#include <QCoreApplication>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTimer>
#include "tmtbConnector.h"
#include "rzz71.h"

const QString DEFAULT_CONFIG_FILENAME = "config.json";

struct ConfigNotFound : public std::logic_error {
    ConfigNotFound(const std::string &str) : std::logic_error(str) {}
    ConfigNotFound(const QString &str) : logic_error(str.toStdString()) {}
};

struct FileWriteError : public std::logic_error {
    FileWriteError(const std::string &str) : std::logic_error(str) {}
    FileWriteError(const QString &str) : logic_error(str.toStdString()) {}
};

struct JsonParseError : public std::logic_error {
    JsonParseError(const std::string &str) : std::logic_error(str) {}
    JsonParseError(const QString &str) : logic_error(str.toStdString()) {}
};

enum class StartupError {
    Ok = 0,
    ConfigLoad = 1,
    RZZStart = 2,
};

class DaemonCoreApplication : public QCoreApplication {
    Q_OBJECT
public:
    DaemonCoreApplication(int &argc, char **argv);
    ~DaemonCoreApplication() override = default;

    StartupError startupError() const { return startError; }
private:
    TRZZ71 *rzz;
    QJsonObject config;
    QString configFileName;

    StartupError startError = StartupError::Ok;

    void loadConfig(const QString &filename);
    void saveConfig(const QString &filename);

};

#endif // MAIN_H
