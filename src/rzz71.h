#ifndef RZZ71_H
#define RZZ71_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "logging.h"
#include "rzz/obecne.h"
#include "rzz/blokV.h"
#include "rzz/cesty.h"

class TRZZ71 : public QObject
{
    Q_OBJECT
public:
    explicit TRZZ71(QObject *parent = nullptr);


private:
    QTimer blik;
    QTimer tim_eval;
    QElapsedTimer timer;
    //QList<mtbpin> blikpin;
    //bool blikOut;
    //void blikpinAdd(mtbpin p);
    //void blikpinRemove(mtbpin p);
    mtbpin pinKPV;
    Tblok* findBlokByName(QString name);

signals:
    void setOutput(int addr, int pin, int state);
    void subscribeModule(int addr);

public slots:
    void getInput(int addr, int pin, int state);
    void init();

private slots:
    void onblik();
    void oneval();
};

#endif // RZZ71_H
