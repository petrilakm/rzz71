#include "tmtbConnector.h"
#include "logging.h"

/*
TmtbConnector

ToDo:
- tcpsocket přehodit sem
- vše s MTB bude uvnitř konektoru (zde)
- vytvořit novou class pro RZZ, ta bude používat konektor a logger, nic víc

*/

TmtbConnector mtb;
//TmtbConnector mtb_stanice;

TmtbConnector::TmtbConnector(QObject *parent)
    : QObject{parent}
{
    connect(&tcpSocket, SIGNAL(getModuleStateIn(QJsonObject)),  this, SLOT(getModuleStateIn(QJsonObject)));
    //connect(&tcpSocket, SIGNAL(getModuleStateOut(QJsonObject)), this, SLOT(getModuleStateOut(QJsonObject)));
    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(getConnected()));
    connect(&tcpSocket, SIGNAL(disconnected()), this, SLOT(getDisconnected()));
    // set reconnect timer
    connect(&t_reconnect, SIGNAL(timeout()), this, SLOT(tReconnectTick()));
    t_reconnect.setSingleShot(true);
    // wait for config to continue (loadConfig)
}

void TmtbConnector::loadConfig(const QJsonObject& config, int servernum) {
    const QJsonObject serverConfig = config["server"+QString::number(servernum)].toObject();
    this->serverHost = serverConfig["host"].toString();
    this->serverPort = serverConfig["port"].toInt();
    log(QString("nastaven MTB daemon %1 na %2:%3")
            .arg(servernum)
            .arg(this->serverHost)
            .arg(this->serverPort),
        logging::LogLevel::Info);
    t_reconnect.start(300); // init reconect timer
}


void TmtbConnector::tReconnectTick() {
    if (!tcpSocket.isConnected) {
        log(QString("pokus o připojení k mtb-daemon na %1:%2")
                .arg(this->serverHost)
                .arg(this->serverPort),
            logging::LogLevel::Info);
        tcpSocket.doConnect(serverHost, serverPort);
        t_reconnect.start(T_RECONNECT_PERIOD);
    }

}
/*
void TmtbConnector::getModuleStateOut(QJsonObject json)
{
    //
}
*/
void TmtbConnector::getModuleStateIn(QJsonObject json)
{
    int addr;
    if (json.contains("address")) {
        addr = json.value("address").toInt();
        if (addr < 0) return;
        if (addr > 255) return;
    } else {
        log("mtb: odpověď neobsahuje adresu modulu", logging::LogLevel::Warning);
        return;
    }
    bool actual;
    if (json.contains("inputs")) {
        QJsonObject ins = json.value("inputs").toObject();
        if (ins.contains("full")) {
            QJsonArray inarr = ins["full"].toArray();
            int i=0;
            for (const QJsonValueRef & val : inarr) {
                actual = QJsonValue(val).toBool();
                if (module[addr].inputs[i] != actual) {
                    module[addr].inputs[i] = actual;
                    int state = 0;
                    if (actual) state = 1;
                    emit ChangedInput(addr, i, state);
                }
                i++;
            }
        }
    }
}

void TmtbConnector::getConnected()
{
    foreach (int i, futureSubscribe) {
        tcpSocket.subscribeModule(i);
    }
    emit mtbConnected();
}

void TmtbConnector::getDisconnected()
{
    emit mtbDisconnected();
    t_reconnect.start(T_RECONNECT_PERIOD);
}

void TmtbConnector::subscribeModule(int addr)
{
    if (addr>100) addr -= 100;
    log("mtb: pokus o zapsání modulu mtb "+QString::number(addr), logging::LogLevel::Debug);
    if (!futureSubscribe.contains(addr)) futureSubscribe.append(addr);
    if (tcpSocket.isConnected) {
            tcpSocket.subscribeModule(addr);
    }
}

void TmtbConnector::setOutput(int addr, int pin, int state)
{
    if (tcpSocket.isConnected) {
        log(QString("mtb%1: pokus o nastavení výstupu %2/%3 = %4")
                .arg(num)
                .arg(addr)
                .arg(pin)
                .arg(state),
            logging::LogLevel::Debug);
        tcpSocket.setOutputs(addr, pin, state);
    } else {
        log(QString("mtb%1: není připojeno, výstup %2/%3 = %4").arg(num).arg(addr).arg(pin).arg(state), logging::LogLevel::Warning);
    }
}

void TmtbConnector::setOutputScom(int addr, int pin, int state)
{
    if (tcpSocket.isConnected) {
        log(QString("mtb%1: pokus o nastavení Scom výstupu %2/%3 = %4")
                .arg(num)
                .arg(addr)
                .arg(pin)
                .arg(state),
            logging::LogLevel::Debug);
        tcpSocket.setOutputsSCOM(addr, pin, state);
    } else {
        log(QString("mtb%1: není připojeno, výstup %2/%3 = %4").arg(num).arg(addr).arg(pin).arg(state), logging::LogLevel::Warning);
    }
}
