#include "blokEMZ.h"

TblokEMZ::TblokEMZ() {
    typ = btEMZ;
    for (int i = 0; i < RELAY_COUNT_EMZ; ++i) {
        r.append(false);
    }
}

bool TblokEMZ::evaluate()
{
    QList<bool> rLast = r;
    // logika

    // zjístí, zda jso všechny výmeny v základní poloze
    // zjistí, zda nekterá není pod záverem (ToDo: vlakovým )
    r[ZP] = true;
    for(int i = 0; i < vym.count(); i++) {
        r[ZP] &= vym[i]->r[TblokV::rel::DP]; // základní poloha = dohled plusu
    }

    if (!r[UK]) {
        // klíč zapevněn
        if (!r[Z] && mtbIns[mtbInPrevzeti].value()) {
            // pokud nejsou výměny pod zavěrem, umožníme uvolnění klíče
            r[UK] = true;
        }
    } else {
        // klíč uvolněn
        if (r[ZP] && mtbIns[mtbInVraceni].value()) {
            // pokud jsou výměny v základní poloze, umožníme zapevnění klíče
            r[UK] = false;
        }
    }

    // blokuje zamknuté výměny:
    for(int i = 0; i < vym.count(); i++) {
        vym[i]->r[TblokV::rel::BP] = !r[UK];
    }

    // reší závěr přes závěrné úseky
    r[Z] = false;
    for (Tblok *b : odvratneBloky) {
        if (b->typ == btS) {
            r[Z] |= b->r[TblokS::rel::Z];
            if (!b->r[TblokS::rel::Z]) odvratneBloky.removeOne(b);
        }
    }

    mtbOut[mtbOutIndikace].setValueBool(r[UK] && (rBlik50 || (!r[ZP])));

    if (r != rLast) return true; else return false;
}
