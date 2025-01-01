#ifndef BLOKKU_H
#define BLOKKU_H

#include <QObject>
#include "rzz/blok.h"

class TblokKU : public Tblok
{
    Q_OBJECT
public:
    explicit TblokKU(QObject *parent = nullptr);

    enum mtbeIns {
        mtbInObsaz = 0,
    };
    enum mtbeOut {
        mtbOutIndBila = 0,
        mtbOutIndCervena = 1,
        mtbOutZvonec = 2,
    };

    bool evaluate() override;

    enum rel {J, OC, EVO, Zv, ZP};
    #define RELAY_COUNT_KU (4)
    // J - obsazeno
    // OC - odjezdová cesta  -> nezvonit při obsazení
    // EVO - evidence odjezdu - zda už se odjelo
    // Zv - zvonek
    // ZP - zvonek protiopakovací

private:
    QTimer *timZvonek; // zvoní zvoncem
private slots:
    void slotZvonekKonec();
};

#endif // BLOKKU_H
