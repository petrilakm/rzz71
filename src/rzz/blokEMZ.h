#ifndef BLOKEMZ_H
#define BLOKEMZ_H

#include "rzz/blok.h"
#include "rzz/blokV.h"

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

    enum rel {UK,ZP,Z};
    #define RELAY_COUNT_EMZ (3)
    // UK - uvolnění klíče
    // ZP - základní poloha výměn
    // Z - závěr aspon jedné z výměn

    QList<TblokV*> vym; // výměny ovládané zámkem
    QList<Tblok *> odvratneBloky; // seznam bloků, kterým tento zámek tvoří odvrat
};

#endif // BLOKEMZ_H
