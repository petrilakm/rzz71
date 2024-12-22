#include "blokK.h"
#include "blokS.h"

TblokK::TblokK() {
    typ = btK;
    for (int i = 0; i < RELAY_COUNT_K; ++i) {
        r.append(false);
    }
    predBlok1 = NULL;
    predBlok2 = NULL;
}

bool TblokK::evaluate()
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

    bool bBila         = (!r[J]) && (r[X1] || r[X2]);
    bool bCervenaKraj  = ( r[J]) && (r[X1] || r[X2]);
    bool bCervenaStred = ( r[J]);

    if (predBlok1) {
        if (predBlok1->typ == btS) {
            r[X1] = predBlok1->r[TblokS::Z];
        }
    }
    if (!r[X1]) predBlok1 = NULL;
    if (predBlok2) {
        if (predBlok2->typ == btS) {
            r[X2] = predBlok2->r[TblokS::Z];
        }
    }
    if (!r[X2]) predBlok2 = NULL;

    // bez výluky nemůže být aktivní K1, K2 (konec posunové cesty)
    if (!r[X1]) r[K1] = false;
    if (!r[X2]) r[K2] = false;

    mtbOut[mtbOutCervenaStred].setValueBool(bCervenaStred);
    mtbOut[mtbOutCervenaKraje].setValueBool(bCervenaKraj);
    mtbOut[mtbOutBila].setValueBool(bBila);

    if (r != rLast) return true; else return false;
}
