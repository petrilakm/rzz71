#pragma once

#include <QObject>
#include <QThread>
#include <iostream>

#ifdef Q_OS_WIN
#include <QWinEventNotifier>
#include <windows.h>
#else
#include <QSocketNotifier>
#endif

class QConsoleListener : public QObject
{
    Q_OBJECT

public:
    QConsoleListener();
    ~QConsoleListener();

signals:
	// connect to "newLine" to receive console input
    void newLine(const QString &strNewLine);
	// finishedGetLine if for internal use
	void finishedGetLine(const QString &strNewLine);

private:
#ifdef Q_OS_WIN
    QWinEventNotifier *m_notifier;
#else
    QSocketNotifier *m_notifier;
#endif

private slots:
	void on_finishedGetLine(const QString &strNewLine);

private:
	QThread m_thread;
};
