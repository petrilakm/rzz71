#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QIODevice>
#include "main.h"
#include "logging.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_WIN
static BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType);
#endif

QConsoleListener *console;

int main(int argc, char *argv[])
{
    console = new QConsoleListener;
    DaemonCoreApplication a(argc, argv);
#ifdef Q_OS_WIN
    SetConsoleCtrlHandler(console_ctrl_handler, TRUE);
#endif
    if (a.startupError() != StartupError::Ok)
        return static_cast<int>(a.startupError());
    return a.exec();
    /*
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "RZZ71_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    */
}

#ifdef Q_OS_WIN
// Handler function will be called on separate thread!
static BOOL WINAPI console_ctrl_handler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT: // Ctrl+C
        QCoreApplication::quit();
        return TRUE;
    case CTRL_CLOSE_EVENT: // Closing the console window
        QCoreApplication::quit();
        return TRUE;
    }

    // Return TRUE if handled this message, further handler functions won't be called.
    // Return FALSE to pass this message to further handlers until default handler calls ExitProcess().
    return FALSE;
}
#endif

const QJsonObject DEFAULT_CONFIG = {
    {"loglevel", static_cast<int>(logging::LogLevel::Info)},
    {"server", QJsonObject{
                   {"host", "127.0.0.1"},
                   {"port", static_cast<int>(SERVER_DEFAULT_PORT)},
               }},
    {"production_logging", QJsonObject{
                               {"enabled", false},
                               {"loglevel", static_cast<int>(logging::LogLevel::RawData)},
                               {"history", 100},
                               {"future", 20},
                               {"directory", "prodLog"},
                               {"detectLevel", static_cast<int>(logging::LogLevel::Warning)},
                               }},
    };

DaemonCoreApplication::DaemonCoreApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv)
{
    // connect signals
#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
#endif

    log("Start RZZ71 v"+QString(VERSION)+"...", logging::LogLevel::Info);
    this->configFileName = (argc > 1) ? argv[1] : DEFAULT_CONFIG_FILENAME;
    try {
        this->loadConfig(this->configFileName);
        log("Config file "+configFileName+" načten.", logging::LogLevel::Info);
    } catch (const ConfigNotFound&) {
        log("Nelze načíst konfigurační soubor "+configFileName+", používá se výchozí nastavení, které se ihned uloží...",
            logging::LogLevel::Info);
        this->config = DEFAULT_CONFIG;
        this->saveConfig(configFileName);
    } catch (const JsonParseError& e) {
        log(e.what(), logging::LogLevel::Error);
        startError = StartupError::ConfigLoad;
        return;
    }

    // config to other modules
    logger.loadConfig(this->config);
    mtb.num = 1;
    mtb_stanice.num = 2;
    mtb.loadConfig(this->config, 1);
    mtb_stanice.loadConfig(this->config, 2);
    rzz = new TRZZ71(this);

    connect(&mtb, SIGNAL(ChangedInput(int,int,int)), rzz, SLOT(getInput(int,int,int)));
    connect(&mtb_stanice, SIGNAL(ChangedInput(int,int,int)), rzz, SLOT(getInput(int,int,int)));
    connect(rzz, SIGNAL(setOutput(int,int,int)), &mtb, SLOT(setOutput(int,int,int)));
    connect(rzz, SIGNAL(setOutput(int,int,int)), &mtb_stanice, SLOT(setOutput(int,int,int)));
    connect(rzz, SIGNAL(subscribeModule(int)), &mtb, SLOT(subscribeModule(int)));
    connect(rzz, SIGNAL(subscribeModule(int)), &mtb_stanice, SLOT(subscribeModule(int)));

    connect(console, SIGNAL(newLine(QString)), rzz, SLOT(readCommand(QString)));
    rzz->init();

}

void DaemonCoreApplication::loadConfig(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw ConfigNotFound(QString("Configuration file not found!"));
    QString content = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &parseError);
    if (doc.isNull())
        throw JsonParseError("Unable to parse config file "+filename+": "+parseError.errorString()+" offset: "+QString::number(parseError.offset));
    this->config = doc.object();
}

void DaemonCoreApplication::saveConfig(const QString &filename) {
    log("Saving config to "+filename+"...", logging::LogLevel::Info);

    QJsonObject root = this->config;
    QJsonDocument doc(root);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw FileWriteError("Unable to open "+filename+" for writing!");
    file.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
    file.close();
}

