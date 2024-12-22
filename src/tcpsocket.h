#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include "MtbModules.h"

class tcpsocket : public QObject
{
    Q_OBJECT
public:
    explicit tcpsocket(QObject *parent = nullptr);

    void doConnect(QString addr, int port);
    void doDisconnect();
    void getModuleList();
    void getModuleInfo(int module);
    void setOutputs(int module, int port, int state);
    void setOutputsSCOM(int module, int port, int state);
    void getOutputs(int module);
    void subscribeModule(int addr);
    void unsubscribeModule(int arrd);
    void setServoOuts(int module, int servo, int state);
    void setServoManual(int module, int servo, uint8_t position);
    void setServoManualEnd(int module);
    void reboot(int module);
    void loadconfig(void);
    void saveconfig(void);
    void upgrade_fw(int module, QString filename);
    bool isConnected = false;

    QList<TMtbModuleState> modules;

    void setModuleName(int module, QString newname);
    void setModuleType(int module, int type);
    void setModuleAddress(int oldAddress, int newAddress);

signals:
    void getOutputsResponse();
    void connected();
    void disconnected();
    void getModuleStateOut(QJsonObject json);
    void getModuleStateIn(QJsonObject json);
    void responseModuleInfo();
    void responseModuleList();

public slots:
    void socket_connected();
    void socket_disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    QTcpSocket *socket;
    //QTimer *tim;
    int id;
    void sendJson(QJsonObject json);
    void parseModuleList(QJsonObject json);
};

#endif // TCPSOCKET_H
