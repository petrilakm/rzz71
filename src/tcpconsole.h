#ifndef TCPCONSOLE_H
#define TCPCONSOLE_H

#include <QObject>
#include <QTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "logging.h"

class Tclient {
public:
    QTcpSocket *socket;
    logging::LogLevel loglevel;
};

class Ttcpconsole : public QObject
{
    Q_OBJECT
public:
    explicit Ttcpconsole(QObject *parent = nullptr);
    void doConnect(QString addr, int port);
    void doDisconnect();
    bool isListening = false;

signals:
    void newLine(QString);

private slots:
    void newClient();
    void disconnectedClient();
    void readyRead();
public slots:
    void sendMsg(QString msg);
    void sendEvent(QString msg, logging::LogLevel loglevel);
private:
    QTcpServer *server;
    QTimer *tim;

    QList<Tclient *> clients;
    void sendMsgToSocket(QTcpSocket *socket, QString msg);
    //QList<QTcpSocket*> clients;
};

#endif // TCPCONSOLE_H
