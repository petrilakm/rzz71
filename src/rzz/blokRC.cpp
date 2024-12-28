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
    bool bCestaNaZruseni = false;

    if (!config.cfgVybav) {
        for(TdohledCesty::cestaPodDohledem *c : dohledCesty.cestyPostavene) {
            if (cestyRC.contains(c->num)) {
                // staráme se o postavenou cestu
                if (c->stav == TdohledCesty::scProjeto) {
                    //r[rel::EV] = true;
                    bCestaNaZruseni = true;
                    if (mtbIns[mtbInRC].value()) {
                        // stisknuté tlačítko
                        r[rel::EV] = false;
                        if (!dohledCesty.cestyNaVybaveni.contains(c)) {
                            dohledCesty.cestyNaVybaveni.append(c);
                        }
                    }
                }
            }
        }
    }
    r[rel::EV] = bCestaNaZruseni;

    // výstup na indikaci RC
    mtbOut[mtbOutRC].setValueBool(r[rel::EV] && rBlik50);

    if (r != rLast) return true; else return false;
}
