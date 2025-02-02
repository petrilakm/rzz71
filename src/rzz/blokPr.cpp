#include "blokPr.h"
#include "blokS.h"
#include "blokV.h"

// Blok průsvitky

TblokPr::TblokPr(QObject *parent) : Tblok{parent}
{
    typ = btPr;
    for (int i = 0; i < RELAY_COUNT_Pr; ++i) {
        r.append(false);
    }
}

bool TblokPr::evaluate()
{
    QList<bool> rLast = r;
    // logika
    if (predBlok) {
        if (predBlok->typ == btV) {
            if (predBlokMinus) {
                r[PrB] = predBlok->r[TblokV::rel::PBM];
                r[PrC] = predBlok->r[TblokV::rel::PCM];
            } else {
                r[PrB] = predBlok->r[TblokV::rel::PBP];
                r[PrC] = predBlok->r[TblokV::rel::PCP];
            }

        }
         if (predBlok->typ == btS) {
            r[PrB] = predBlok->r[TblokS::rel::PrB];
            r[PrC] = predBlok->r[TblokS::rel::PrC];
        }
    } else {
        r[PrB] = false;
        r[PrC] = rBlik50;
    }

    mtbOut[mtbOutPrusvBila].setValueBool(r[PrB]);
    mtbOut[mtbOutPrusvCervena].setValueBool(r[PrC]);

    if (r != rLast) return true; else return false;
}


