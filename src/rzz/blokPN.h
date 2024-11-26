#ifndef BLOKPN_H
#define BLOKPN_H

#include "rzz/blok.h"
#include "rzz/blokQ.h"
#include "rzz/blokTC.h"

class TblokPN : public Tblok
{
public:
    TblokPN();

    enum mtbeIns {
        mtbInPN = 0,
    };
    enum mtbeOut {
        mtbOutPocitadlo = 0,
    };

    bool evaluate() override;

    enum rel {PN,BT};
    #define RELAY_COUNT_PN (2)
    // PN - připovávací návest sepnuta
    // BT - blokování tlačítka (vytažení u navěstidla)

    TblokQ *navestidlo;
    TblokTC *tlacitkoUNavestidla;
    QTimer *tim;
public slots:
    void on_tim();
};

#endif // BLOKPN_H
