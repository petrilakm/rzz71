#include "blokS.h"

TblokS::TblokS() {
    typ = btS;
    typM = false;
    for (int i = 0; i < RELAY_COUNT_S; ++i) {
        r.append(false);
    }
}

bool TblokS::evaluate()
{
    QList<bool> rLast = r;
    // logika

    if (mtbIns[mtbInObsaz].valid) {
        if (!(rDCCVypadek || rDCCZkrat)) {
            r[J] = mtbIns[mtbInObsaz].value();
        }
    } else {
        r[J] = true;
    }

    r[V] |= (mtbIns[mtbInNuz].value() && (r[Z]) && (!rQTV));
    r[V] &= (r[Z]);
    //r[Z] = r[A] || r[B];
    if (r[V]) {
        r[R] = (rD3V);
    } else {
        r[R] = false; // doplnit
    }
    r[Z] &= !r[R];
    //r[B] &= !r[R];

    if ((r[Z])) {
        r[PrB] = !r[J] && ((r[V]) ? rBlik50 : 1);
    } else {
        r[PrB] = !typM && rKPV;
    }
    r[PrC] = r[J];


//    mtbOut[mtbOutCervena].setValueBool(false);
//    mtbOut[mtbOutBila].setValueBool(false);



    if (r != rLast) return true; else return false;
}
