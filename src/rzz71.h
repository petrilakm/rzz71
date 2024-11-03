#ifndef RZZ71_H
#define RZZ71_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "tcpconsole.h"
#include "logging.h"
#include "rzz/obecne.h"
#include "rzz/blokV.h"
#include "rzz/cesty.h"

class TRZZ71 : public QObject
{
    Q_OBJECT
public:
    explicit TRZZ71(QObject *parent = nullptr);
    // pro LCD modul, aby mohl kontrolovat casové soubory
    QTimer t3V; // 3 min
    QTimer t3C; // 3 min
    QTimer t1C; // 1 min
    QTimer t5C; // 5 s

private:
    Ttcpconsole *tcpcon;
    QTimer blik;
    QTimer tim_eval;
    QElapsedTimer timer;
    bool bFirstRun = false;

    mtbpin pinAmp01;
    mtbpin pinAmp02;
    mtbpin pinAmp04;
    mtbpin pinAmp08;
    mtbpin pinAmp10;
    mtbpin pinAmp20;

    QTimer simul_puls_timer;
    mtbpin simul_puls_pin;
    //QList<mtbpin> blikpin;
    //bool blikOut;
    //void blikpinAdd(mtbpin p);
    //void blikpinRemove(mtbpin p);
    mtbpin pinInKPV;
    mtbpin pinInNUZ;
    mtbpin pinOutNUZ;
    mtbpin pinOutKmitac;
    mtbpin pinInZkrat;
    Tblok* findBlokByName(QString name);

signals:
    void setOutput(int addr, int pin, int state);
    void subscribeModule(int addr);

public slots:
    void getInput(int addr, int pin, int state);
    void init();
    void readCommand(QString);

private slots:
    void onblik();
    void oneval();
    void ont3V();
    void ont3C();
    void ont1C();
    void ont5C();
    void onSimulPuls();
};

#endif // RZZ71_H
