#include "blokK.h"

TblokK::TblokK() {
    typ = btK;
    for (int i = 0; i < RELAY_COUNT_K; ++i) {
        r.append(false);
    }
}

bool TblokK::evaluate()
{
    QList<bool> rLast = r;
    // logika

    r[J] = mtbIns[mtbInObsaz].value();

    bool bBila         = (!r[J]) && (r[X1] || r[X2]);
    bool bCervenaKraj  = ( r[J]) && (r[X1] || r[X2]);
    bool bCervenaStred = ( r[J]);


    mtbOut[mtbOutCervenaStred].setValueBool(bCervenaStred);
    mtbOut[mtbOutCervenaKraje].setValueBool(bCervenaKraj);
    mtbOut[mtbOutBila].setValueBool(bBila);

    if (r != rLast) return true; else return false;
}
