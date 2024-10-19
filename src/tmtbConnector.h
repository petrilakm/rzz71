#ifndef TMTBCONNECTOR_H
#define TMTBCONNECTOR_H

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include "tcpsocket.h"

constexpr size_t T_RECONNECT_PERIOD = 5000; // 5 s
constexpr size_t SERVER_DEFAULT_PORT = 3841;

class TmtbConnector : public QObject
{
    Q_OBJECT
public:
    explicit TmtbConnector(QObject *parent = nullptr);
    void loadConfig(const QJsonObject& config, int servernum);

    int num = 0;

    struct smtbmodule {
        bool inputs[16];
        int outputs[16+2*6];
    };

    struct smtbmodule module[256]; // module states

    struct smtbfutureout {
        uint8_t state;
        uint8_t addr;
        uint8_t pin;
    };

    // API


private:
    bool configLoaded = false;
    QString serverHost;
    int serverPort;
    QTimer t_reconnect;
    tcpsocket tcpSocket;

    QList<int> futureSubscribe;
    QList<smtbfutureout> futureOutputs;

signals:
    void ChangedInput(int addr, int pin, int state); // receive from MTB
    void mtbConnected();
    void mtbDisconnected();

public slots:
    void subscribeModule(int addr);
    void setOutput(int addr, int pin, int state); // send to MTB

private slots:
    // timer
    void tReconnectTick();
    // from daemon
    //void getModuleStateOut(QJsonObject json);
    void getModuleStateIn(QJsonObject json);
    void getConnected();
    void getDisconnected();

signals:

};

extern TmtbConnector mtb;
extern TmtbConnector mtb_stanice;

#endif // TMTBCONNECTOR_H
