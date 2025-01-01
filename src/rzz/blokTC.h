#ifndef BLOKTC_H
#define BLOKTC_H

#include "rzz/blok.h"

class TblokTC : public Tblok
{
    Q_OBJECT
public:
    explicit TblokTC(QObject *parent = nullptr);

    enum mtbeIns {
        mtbInVolba = 0,
        mtbInRuseni = 1,
    };
    enum mtbeOut {
        mtbOutIndikace = 0,
    };

    bool evaluate() override;

    enum rel {TZ, PO, VA, NM, ZFo, RC};
    #define RELAY_COUNT_TC (6)
    // TZ - tlačítko zapínací
    // PO - protiopakovací relé (PV nebo PS)
    // VA - výměnové automacké relé - stavíme výhybky
    // NM - pokud je DN na návěstidle (N nebo M)
    // ZFo - opakovač ZF - blokování rušení - pro přivolávací návěst
    // RC - rušení cesty (počátkem)

    // proti volbě počítku, když se volí konec
    // každý stisk se do volíví skupiny pošle jen 1x
    bool mtbVolbaOpak = false;
};

#endif // BLOKTC_H
