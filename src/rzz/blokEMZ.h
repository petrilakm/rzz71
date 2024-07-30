#ifndef BLOKEMZ_H
#define BLOKEMZ_H

#include "rzz/blok.h"

class TblokEMZ : public Tblok
{
public:
    TblokEMZ();

    enum mtbeIns {
        mtbInPrevzeti = 0,
        mtbInVraceni = 1,
    };
    enum mtbeOut {
        mtbOutIndikace = 0,
    };

    bool evaluate() override;

    enum rel {UK,ZP};
    #define RELAY_COUNT_EMZ (2)
    // UK - uvolnění klíče
    // ZP - základní poloha výměn
};

#endif // BLOKEMZ_H
