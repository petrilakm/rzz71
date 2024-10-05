#ifndef BLOKPN_H
#define BLOKPN_H

#include "rzz/obecne.h"
#include "rzz/blok.h"
#include "rzz/blokQ.h"

class TblokPN : public Tblok
{
public:
    TblokPN();

    enum mtbeIns {
        mtbInPN = 0,
        mtbInPNEnable = 1,
    };
    enum mtbeOut {
        mtbOutPocitadlo = 0,
    };

    bool evaluate() override;

    enum rel {PN};
    #define RELAY_COUNT_PN (1)

    TblokQ *navestidlo;
    QTimer *tim;
public slots:
    void on_tim();
};

#endif // BLOKPN_H
