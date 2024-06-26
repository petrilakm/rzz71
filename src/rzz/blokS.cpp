#include "blokS.h"

TblokS::TblokS() {
    typ = btS;
    typM = false;
    for (int i = 0; i < 16; ++i) {
        r.append(false);
    }
}

bool TblokS::evaluate()
{
    QList<bool> rLast = r;
    // logika

    r[J] = mtbIns[mtbInObsaz].value();
    r[Z] = r[A] || r[B];

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
