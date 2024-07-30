#include <iostream>
#include <QJsonObject>
#include <QDir>
#include "logging.h"
#include "termcolor.h"

Logger logger;

void Logger::loadConfig(const QJsonObject& config) {
    this->loglevel = static_cast<logging::LogLevel>(config["loglevel"].toInt());

	const QJsonObject& prodCfg = config["production_logging"].toObject();
	this->prod.enabled = prodCfg["enabled"].toBool();
	if (this->prod.enabled) {
        this->prod.loglevel = static_cast<logging::LogLevel>(prodCfg["loglevel"].toInt());
		this->prod.history = prodCfg["history"].toInt();
		this->prod.future = prodCfg["future"].toInt();
		this->prod.directory = prodCfg["directory"].toString();
        this->prod.detectLevel = static_cast<logging::LogLevel>(prodCfg["detectLevel"].toInt());

		if (access(this->prod.directory.toStdString().c_str(), W_OK) != 0)
			log("Production logging to '"+this->prod.directory+"' directory set, "
			    "however directory is not writeable/does not exist",
                logging::LogLevel::Warning);
	}
}


void log(const QString& message, logging::LogLevel loglevel) {
	logger.log(message, loglevel);
}

void term(const QString& message) {
    std::cout << message.toStdString() << std::endl;
}

void Logger::log(const QString& message, logging::LogLevel loglevel) {
	this->termLog(message, loglevel);
	if (this->prod.enabled)
		this->prodLog(message, loglevel);
}

void Logger::termLog(const QString& message, logging::LogLevel loglevel) {
	if (loglevel > this->loglevel)
		return;

	switch (loglevel) {
        case logging::LogLevel::Error: std::cout << termcolor::bold << termcolor::red; break;
        case logging::LogLevel::Warning: std::cout << termcolor::bold << termcolor::yellow; break;
        case logging::LogLevel::Info: std::cout << termcolor::bold; break;
        case logging::LogLevel::RawData: std::cout << termcolor::cyan; break;
        case logging::LogLevel::Debug: std::cout << termcolor::magenta; break;
		default: break;
	}

	std::cout << "[" << QTime::currentTime().toString("hh:mm:ss,zzz").toStdString() << "] ";
	switch (loglevel) {
        case logging::LogLevel::Error: std::cout << "[ERROR] "; break;
        case logging::LogLevel::Warning: std::cout << "[WARNING] "; break;
        case logging::LogLevel::Info: std::cout << "[info] "; break;
        case logging::LogLevel::Commands: std::cout << "[command] "; break;
        case logging::LogLevel::RawData: std::cout << "[raw-data] "; break;
        case logging::LogLevel::Debug: std::cout << "[debug] "; break;
		default: break;
	}
	std::cout << message.toStdString() << termcolor::reset << std::endl;
}

void Logger::prodLog(const QString& message, logging::LogLevel loglevel) {
	if (loglevel <= this->prod.loglevel) {
		this->logHistory.emplace_back(message, loglevel);
		if (this->logHistory.size() > this->prod.history)
			this->logHistory.pop_front();
	}

	if ((this->prod.remaining > 0) && (loglevel <= this->prod.loglevel)) {
		if (this->prod.file != nullptr) {
			*(this->prod.file) << this->logHistory.back();
			if (!this->prod.file->is_open())
				this->prod.file = nullptr;
		}
		this->prod.remaining--;
		if (this->prod.remaining == 0)
			this->prod.file = nullptr; // will cause close
	}

	if (loglevel <= this->prod.detectLevel) {
		// activate / another detection
		if (!this->prod.active())
			this->prodInit();
		this->prod.remaining = this->prod.future;
	}
}

void Logger::prodInit() {
    const std::string filename = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss").toStdString() + ".log";
    const std::string path = (this->prod.directory + QDir::separator()).toStdString() + filename;
    this->prod.file = std::make_unique<std::ofstream>(path);
	if (!this->prod.file->is_open()) {
		this->prod.file = nullptr;
		return;
	}

	if (this->prod.file != nullptr)
		for (const LogRecord& record : this->logHistory)
			*(this->prod.file) << record;
}

std::ofstream& operator<<(std::ofstream& os, const LogRecord& record) {
	os << "[" << record.time.toString("hh:mm:ss,zzz").toStdString() << "] ";
	switch (record.loglevel) {
        case logging::LogLevel::Error: os << "[ERROR] "; break;
        case logging::LogLevel::Warning: os << "[WARNING] "; break;
        case logging::LogLevel::Info: os << "[info] "; break;
        case logging::LogLevel::Commands: os << "[command] "; break;
        case logging::LogLevel::RawData: os << "[raw-data] "; break;
        case logging::LogLevel::Debug: os << "[debug] "; break;
		default: break;
	}
	os << record.message.toStdString() << std::endl;
	return os;
}
