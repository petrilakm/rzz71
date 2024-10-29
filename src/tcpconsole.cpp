#include "tcpconsole.h"
#include "logging.h"

Ttcpconsole::Ttcpconsole(QObject *parent)
    : QObject{parent}
{
    server = new QTcpServer;
    server->listen(QHostAddress("0.0.0.0"), 27016);

    connect(server, SIGNAL(newConnection()),this, SLOT(newClient()));
    log(tr("console: OK"), logging::LogLevel::Info);
}

// server handling
void Ttcpconsole::newClient()
{
    QTcpSocket *s;
    s = server->nextPendingConnection();
    connect(s, SIGNAL(disconnected()), this, SLOT(disconnectedClient()));
    connect(s, SIGNAL(readyRead()), this, SLOT(readyRead()));
    log(tr("console: nová konzole %1:%2").arg(s->peerAddress().toString()).arg(s->peerPort()), logging::LogLevel::Info);
    Tclient *cli;
    cli = new Tclient;
    cli->socket = s;
    cli->loglevel = logging::LogLevel::Info;
    clients.append(cli);
}

// socket handling
void Ttcpconsole::disconnectedClient()
{
    QTcpSocket *s = static_cast<QTcpSocket *>(sender());

    log(tr("console: odpojeno %1:%2").arg(s->peerAddress().toString()).arg(s->peerPort()), logging::LogLevel::Info);
    Tclient *cli = nullptr;
    for (Tclient *icli : clients) {
        if (icli->socket == s) break;
    }
    if (cli != nullptr) {
        clients.removeOne(cli);
        delete cli;
    }
    s->close();
    s->deleteLater();
}


void Ttcpconsole::readyRead()
{
    QTcpSocket *s = static_cast<QTcpSocket *>(sender());
    QString line;
    QByteArray ar = s->readAll();
    Tclient *cli = nullptr;
    for (Tclient *icli : clients) {
        if (icli->socket == s) {
            cli = icli;
            break;
        }
    }
    if (cli == nullptr) {
        log(tr("console: neznámý klient poslal data \"%1\" ( sender %2:%3) - len %4").arg(line).arg(s->peerAddress().toString()).arg(s->peerPort()).arg(ar.length()), logging::LogLevel::Info);
        return;
    }
    if (ar.endsWith(13)) ar.removeLast();
    if (ar.endsWith(10)) ar.removeLast();
    if (ar.endsWith(13)) ar.removeLast();
    if (ar.endsWith(10)) ar.removeLast();
    line = QString::fromUtf8(ar);
    //log(tr("console: data \"%1\" ( sender %2:%3) - len %4").arg(line).arg(s->peerAddress().toString()).arg(s->peerPort()).arg(ar.length()), logging::LogLevel::Info);
    // zpracuje příkazy konzole
    if (line == "list") {
        for(Tclient *cli : clients) {
            if (cli->socket == s) {
                sendMsgToSocket(s, tr("klient %1:%2 - to jsme my").arg(cli->socket->peerAddress().toString()).arg(cli->socket->peerPort()));
            } else {
                sendMsgToSocket(s, tr("klient %1:%2").arg(cli->socket->peerAddress().toString()).arg(cli->socket->peerPort()));
            }
        }
        return;
    }
    if (line.startsWith("loglevel")) {
        QStringList chunks = line.split(' ');
        if (chunks.length() == 1) {
            sendMsgToSocket(s, tr("aktuální loglevel = %1").arg((int) cli->loglevel));
        }
        if (chunks.length() == 2) {
            bool ok;
            int num = QString(chunks[1]).toInt(&ok);
            if (ok) {
                cli->loglevel = (logging::LogLevel) num;
                sendMsgToSocket(s, tr("nastaven loglevel = %1").arg((int) cli->loglevel));
            } else {
                sendMsgToSocket(s, tr("neplatný loglevel"));
            }
        }
        return;
    }
    if ((line == "close") || (line == "quit") || (line == "exit")) {
        //sendMsgToSocket(s, tr("quit"));
        s->disconnectFromHost();
        return;
    }

    // předá na zpracování pro RZZ
    emit newLine(line);
}

void Ttcpconsole::sendMsg(QString msg)
{
    for(Tclient *cli : clients) {
        sendMsgToSocket(cli->socket, msg);
    }
}

void Ttcpconsole::sendEvent(QString msg, logging::LogLevel loglevel)
{
    for(Tclient *cli : clients) {
        if (loglevel <= cli->loglevel) {
            sendMsgToSocket(cli->socket, msg);
        }
    }
}

void Ttcpconsole::sendMsgToSocket(QTcpSocket *socket, QString msg)
{
    msg = msg + QChar(13) + QChar(10);
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(QString(msg).toUtf8());
    }
}
