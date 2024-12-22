#include "tcpsocket.h"
#include "logging.h"

tcpsocket::tcpsocket(QObject *parent)
    : QObject{parent}
{
    isConnected = false;
//    tim = new QTimer(this);
}

void tcpsocket::doConnect(QString addr, int port)
{
    if (socket != NULL) {
        log("socket: delete", logging::LogLevel::Info);
        delete socket;
        disconnect(this);
    }
    log("socket: new", logging::LogLevel::Info);
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(connected()),this, SLOT(socket_connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(socket_disconnected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()),this, SLOT(readyRead()));

    log("socket: připojování...", logging::LogLevel::Debug);

    //tim->setInterval(1000);

    this->id = 0;

    // this is not blocking call
    socket->connectToHost(addr, port, QAbstractSocket::ReadWrite, QAbstractSocket::IPv4Protocol);

    // we need to wait...
    if(!socket->waitForConnected(1500))
    {
        log("socket: Chyba: "+socket->errorString(), logging::LogLevel::Error);
    }
}

void tcpsocket::doDisconnect()
{
    if (isConnected) {
        isConnected = false;
        socket->disconnect();
        delete socket;
        modules.clear();
    }
}

void tcpsocket::socket_connected()
{
    log("socket: připojeno", logging::LogLevel::Debug);
    isConnected = true;
    emit connected();

    // Hey server, tell me about you.
    //socket->write("HEAD / HTTP/1.0\r\n\r\n\r\n\r\n");
}

void tcpsocket::socket_disconnected()
{
    log("socket: odpojeno", logging::LogLevel::Debug);
    emit disconnected();
    isConnected = false;
}

void tcpsocket::bytesWritten(qint64 bytes)
{
    log(QString("socket: data zapsány (%1 bytes)").arg(bytes), logging::LogLevel::Debug);
}

void tcpsocket::readyRead()
{
    static QByteArray buf;
    QJsonDocument jsondoc;

    buf.append(socket->readAll());
    if (buf.contains('\n')) {
        QList<QByteArray> chunks = buf.split('\n');
        buf.clear();
        if (!chunks.last().endsWith('\n'))
            buf.append(chunks.takeLast()); // leave only last chunk
        foreach (QByteArray chunk, chunks) {
            if (chunk.length() > 5) { // ignore short messages
                //qDebug() << "found:";
                //qDebug() << qPrintable(chunk);
                jsondoc = QJsonDocument::fromJson(chunk);
                if (jsondoc.isObject() == false) {
                    log("socket: přijat špatný JSON, nelze zpracovat", logging::LogLevel::Warning);
                    continue;
                }
                QJsonObject lvl1 = jsondoc.object();
                if (lvl1.contains("command")) {
                    if (lvl1["command"] == "module_set_outputs") {
                        if (lvl1.contains("outputs"))
                            emit getModuleStateOut(lvl1);
                    }

                    if (lvl1["command"] == "modules") {
                        if (lvl1.contains("modules"))
                            parseModuleList(lvl1["modules"].toObject());
                    }
                    if (lvl1["command"] == "module") {
                        // informace o modulu po připojení k MTB
                        if (lvl1.contains("module")) {
                            uint8_t  addr = 0;
                            QJsonObject lvl2 = lvl1["module"].toObject();
                            if (lvl2.contains("address")) {
                                addr = lvl2["address"].toInt();
                            }
                            if (lvl2.constBegin()->isObject()) {
                                QJsonObject lvl3 = lvl2.constBegin()->toObject();
                                if (lvl3.contains("state")) {
                                    QJsonObject lvl4 = lvl3["state"].toObject();
                                    lvl4.insert("address", addr); // doplní do dat chybějící adresu
                                    emit getModuleStateIn(lvl4);
                                }
                            }
                        }
                    }

                    if (lvl1["command"] == "module_outputs_changed") {
                        log(QString("socket: get module_outputs_changed"), logging::LogLevel::Debug);
                        if (lvl1.contains("module_outputs_changed")) {
                            QJsonObject lvl2 = lvl1["module_outputs_changed"].toObject();
                            emit getModuleStateOut(lvl2);
                        }
                    }
                    if (lvl1["command"] == "module_inputs_changed") {
                        log(QString("socket: get module_inputs_changed"), logging::LogLevel::Debug);
                        if (lvl1.contains("module_inputs_changed")) {
                            QJsonObject lvl2 = lvl1["module_inputs_changed"].toObject();
                            emit getModuleStateIn(lvl2);
                        }
                    }
                }
            }
        }
        //qDebug() << "buf length: "+QString::number(buf.length());
    }
}

void tcpsocket::parseModuleList(QJsonObject json)
{
    qDebug("socket: parse module list");
    modules.clear();
    QStringList modkeys = json.keys();
    for (int i = 0; i < modkeys.count(); i++) {
        TMtbModuleState ms;
        QString modaddr = modkeys[i];
        QJsonObject mod = json[modaddr].toObject();
        QString modname = mod["name"].toString();
        int modtype = mod["type_code"].toInt();
        bool modstate = mod["state"] == QString("active");
        qDebug("socket: module add %s", modaddr.toStdString().c_str());
        ms.address = modaddr.toInt();
        ms.name = modname;
        ms.type = modtype;
        ms.active = modstate;
        modules.append(ms);
    }
    emit responseModuleList();
}

void tcpsocket::getModuleList()
{
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("modules");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["state"] = QJsonValue(false);
    sendJson(tmpObj);
}

void tcpsocket::getModuleInfo(int module)
{
    QJsonArray tmpArr;
    tmpArr.append(QJsonValue(module));
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["address"] =  tmpArr;
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["state"] = QJsonValue(false);
    sendJson(tmpObj);
}

void tcpsocket::subscribeModule(int addr)
{
    /*
{
    "command": "module_subscribe"/"module_unsubscribe",
    "type": "request",
    "id": 12,
    "addresses": [10, 11, 20]
}
*/
    log("socket: zapsání modulu mtb "+QString::number(addr), logging::LogLevel::Debug);
    QJsonArray tmpArr;
    tmpArr.append(QJsonValue(addr));
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module_subscribe");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["addresses"] =  tmpArr;
    tmpObj["id"] = QJsonValue(this->id++);
    sendJson(tmpObj);
    getOutputs(addr); // get state now
}

void tcpsocket::unsubscribeModule(int addr)
{
    log("socket: odvolání zapsání modulu mtb "+QString::number(addr), logging::LogLevel::Debug);
    QJsonArray tmpArr;
    tmpArr.append(QJsonValue(addr));
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module_unsubscribe");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["addresses"] =  tmpArr;
    tmpObj["id"] = QJsonValue(this->id++);
    sendJson(tmpObj);
}

void tcpsocket::getOutputs(int module)
{
    QJsonObject tmpObj;
    tmpObj["command"] = QJsonValue("module");
    tmpObj["type"] = QJsonValue("request");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["state"] = QJsonValue(true);
    tmpObj["id"] = QJsonValue(this->id++);

    sendJson(tmpObj);
    return;
}

void tcpsocket::setOutputs(int module, int port, int state)
{
    QJsonObject tmpOut;
    QJsonObject tmpOutOne;

    log(QString("socket: nastav výstup %1/%2 = %3").arg(module).arg(port).arg(state), logging::LogLevel::Debug);

    tmpOutOne["type"] = QJsonValue("plain");
    tmpOutOne["value"] = QJsonValue(state);
    tmpOut[QString::number(port)] = tmpOutOne;

    QJsonObject tmpObj;
    tmpObj["type"] = "request";

    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_set_outputs");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["outputs"] = tmpOut;
    tmpObj["state"] = QJsonValue(false);

    sendJson(tmpObj);
    return;
}

void tcpsocket::setServoOuts(int module, int servo, int state)
{
    QJsonObject tmpOut;
    QJsonObject tmpOutOne;
    int out1 = 16+(servo-1)*2;
    int out2 = out1 + 1;
    int state1 = (state) ? 0 : 1; // inversion, state 1 = first output 0, second output 1
    int state2 = 1-state1;

    // output states
    tmpOutOne["type"] = QJsonValue("plain");
    tmpOutOne["value"] = QJsonValue(state1);
    tmpOut[QString::number(out1)] = tmpOutOne;
    tmpOutOne["type"] = QJsonValue("plain");
    tmpOutOne["value"] = QJsonValue(state2);
    tmpOut[QString::number(out2)] = tmpOutOne;

    // main json
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_set_outputs");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["outputs"] = tmpOut;
    tmpObj["state"] = QJsonValue(false); // dont report states

    sendJson(tmpObj);
}

void tcpsocket::setServoManual(int module, int servo, uint8_t position)
{
    // main json
    QJsonObject tmpObj;
    QJsonArray tmpArr;

    tmpArr.push_back(QJsonValue(3));
    tmpArr.push_back(QJsonValue(servo << 1));
    tmpArr.push_back(QJsonValue(position));

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_specific_command");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["data"] = tmpArr;

    sendJson(tmpObj);
}

void tcpsocket::setServoManualEnd(int module)
{
    // main json
    QJsonObject tmpObj;
    QJsonArray tmpArr;

    tmpArr.push_back(QJsonValue(3));
    tmpArr.push_back(QJsonValue(0));

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_specific_command");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["data"] = tmpArr;

    sendJson(tmpObj);
}

void tcpsocket::reboot(int module)
{
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_reboot");
    tmpObj["address"] = QJsonValue(module);

    sendJson(tmpObj);
}

void tcpsocket::loadconfig(void)
{
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("load_config");

    sendJson(tmpObj);
}

void tcpsocket::saveconfig(void)
{
    QJsonObject tmpObj;
    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("save_config");

    sendJson(tmpObj);
}

void tcpsocket::upgrade_fw(int module, QString filename)
{
    QJsonObject tmpObj;
    QJsonObject firmware;

    int type;
    int addr;
    int offset = 0;

    // ":100000000C9446010C9465010C9465010C946501F7"
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            type = line.mid(7, 2).toInt(nullptr, 16);
            addr = offset + line.mid(3, 4).toInt(nullptr, 16);
            qDebug() << line.mid(3, 4) + " - " + QString::number(addr);
            if (type == 2) { // change address
                offset = line.mid(9, 4).toInt(nullptr, 16) << 4;
            }
            if (type == 0) { // data
                line.remove(0, 9).chop(2);
                firmware.insert(QString::number(addr), QJsonValue(line));
            }
        }
        inputFile.close();
    } else {
        // file not found
    }

    tmpObj["type"] = "request";
    tmpObj["id"] = QJsonValue(this->id++);
    tmpObj["command"] = QJsonValue("module_upgrade_fw");
    tmpObj["address"] = QJsonValue(module);
    tmpObj["firmware"] = firmware;

    sendJson(tmpObj);
}

void tcpsocket::sendJson(QJsonObject json)
{
    if (isConnected) {
        QJsonDocument tmpJson(json);
        QByteArray req = tmpJson.toJson(QJsonDocument::Compact);
        log("socket: raw=" + QString(qPrintable(req)), logging::LogLevel::RawData);
        socket->write(req+QByteArray("\n\n"));
    }
}
