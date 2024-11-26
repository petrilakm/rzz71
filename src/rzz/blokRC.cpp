#include "blokRC.h"
#include "dohledcesty.h"

TblokRC::TblokRC()
{
    typ = btRC;
    for (int i = 0; i < RELAY_COUNT_RC; ++i) {
        r.append(false);
    }
    cestyRC.clear();
}

bool TblokRC::evaluate()
{
    QList<bool> rLast = r;
    r[rel::EV];
    for(TdohledCesty::cestaPodDohledem *c : dohledCesty.cestyPostavene) {
        if (cestyRC.contains(c->num)) {
            // staráme se o postavenou cestu
            if (c->stav == TdohledCesty::scProjeto) {
                r[rel::EV] = true;
                if (mtbIns[mtbInRC].value()) {
                    // stisknuté tlačítko
                    if (!dohledCesty.cestyNaVybaveni.contains(c)) {
                        dohledCesty.cestyNaVybaveni.append(c);
                    }
                }
            }
        }
    }

    // výstup na indikaci RC
    mtbOut[mtbOutRC].setValueBool(r[rel::EV] && rBlik50);

    if (r != rLast) return true; else return false;
}
