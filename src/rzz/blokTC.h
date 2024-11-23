#ifndef BLOKTC_H
#define BLOKTC_H

#include "rzz/blok.h"

class TblokTC : public Tblok
{
public:
    TblokTC();

    enum mtbeIns {
        mtbInVolba = 0,
        mtbInRuseni = 1,
    };
    enum mtbeOut {
        mtbOutIndikace = 0,
    };

    bool evaluate() override;

    enum rel {TZ, PO, VA, NM};
    #define RELAY_COUNT_TC (4)
    // TK - tlačítko volba
    // PO - protiopakovací relé
    // TZ - trvalé svícení

    // TZ - tlačítko zapínací
    // PO - protiopakovací relé (PV nebo PS)
    // VA - výměnové automacké relé - stavíme výhybky
    // NM - pokud je DN na návěstidle (N nebo M)


    // proti volbě počítku, když se volí konec
    // každý stisk se do volíví skupiny pošle jen 1x
    bool mtbVolbaOpak = false;
};

#endif // BLOKTC_H
