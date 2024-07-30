#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <deque>
#include <fstream>
#include <memory>
#include <QString>
#include <QDateTime>

namespace logging {

enum class LogLevel {
    None = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Commands = 4,
    RawData = 5,
    Debug = 6,
};

}

void log(const QString&, logging::LogLevel);

void term(const QString&);

struct LogRecord {
	QDateTime time;
	QString message;
    logging::LogLevel loglevel;

    LogRecord(const QString& message, logging::LogLevel loglevel)
    : time(QDateTime::currentDateTime()), message(message), loglevel(loglevel) {}
};

std::ofstream& operator<<(std::ofstream&, const LogRecord&);

class Logger {
public:
	void loadConfig(const QJsonObject& config);
    void log(const QString&, logging::LogLevel);

private:
	struct Prod {
		bool enabled = false;
        logging::LogLevel loglevel;
		size_t history;
		size_t future;
		QString directory;
        logging::LogLevel detectLevel;

		size_t remaining = 0;
		std::unique_ptr<std::ofstream> file;
		bool active() const { return this->remaining > 0; }
	};

    logging::LogLevel loglevel = logging::LogLevel::Info;
	Prod prod;
	std::deque<LogRecord> logHistory;

    void prodLog(const QString&, logging::LogLevel);
	void prodInit();
    void termLog(const QString&, logging::LogLevel);
};

extern Logger logger;

#endif
