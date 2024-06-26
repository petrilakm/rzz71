#include "blokQ.h"

TblokQ::TblokQ() {
    typ = btQ;
    for (int i = 0; i < RELAY_COUNT_Q; ++i) {
        r.append(false);
    }
}

bool TblokQ::evaluate()
{
    QList<bool> rLast = r;
    // logika

    //r[J] = mtbIns[mtbInObsaz].value();

    //mtbOut[mtbOutCervena].setValueBool(false);
    //mtbOut[mtbOutBila].setValueBool(false);



    if (r != rLast) return true; else return false;
}
