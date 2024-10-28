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

    enum rel {TK, PO, TZ};
    #define RELAY_COUNT_TC (3)
    // TK - tlačítko volba
    // PO - protiopakovací relé
    // TZ - trvalé svícení

    // proti volbě počítku, když se volí konec
    // každý stisk se do volíví skupiny pošle jen 1x
    bool mtbVolbaOpak = false;
};

#endif // BLOKTC_H
