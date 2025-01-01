#include "voliciskupina.h"
#include "dohledcesty.h"

Tvoliciskupina::Tvoliciskupina()
{
    probihaVolba = false;
}

void Tvoliciskupina::ruseniVolbyCesty()
{
    cestyMozne.clear();
    foreach (TblokTC * tl, tlacitkaAktivni) {
        tl->r[TblokTC::TZ] = false;
        tl->r[TblokTC::PO] = false;
        //tl->r[TblokTC::VA] = false;
    }
    tlacitkaAktivni.clear();
    probihaVolba = false;
}

bool Tvoliciskupina::vstupZmena(TblokTC *p, bool state)
{
    bool ret = false;
    if (state) {
        //log(QString("cesty: změna tlačítka %1").arg(p->name), logging::LogLevel::Info);
        if (tlacitkaAktivni.isEmpty() && (!probihaVolba)) {
            // první tlačítko cesty
            // nejprve ověříme, zda už odsud cesta nevede, která má řešit tlačítko
            for (TdohledCesty::cestaPodDohledem *cdoh : dohledCesty.cestyPostavene) {
                if (cdoh->pCesta->tlacitka.first() == p) {
                    // je postavená cesta od tohoto tlačítka
                    if ((cdoh->stav == TdohledCesty::scDN) || (cdoh->kontrolaCelistvostiCesty())) {
                        // neděláme nic - obnovu DN řeší dohledcesty
                        return false;
                    }
                }
            }
            // zjístíme, jaké cesty u tohoto tlačítka začínají
            cestyMozne.clear();
            for (Tcesta *i : cesty->cesty) {
                //log(QString("cesty: hledání %1 == %2").arg(p->name).arg(i->tlacitka.at(0)->name), logging::LogLevel::Info);
                if (i->tlacitka[0] == p) {
                    cestyMozne.append(i->num);
                    //log(QString("cesty: nalezeno %1").arg(i->num), logging::LogLevel::Info);
                }
            }
            if (cestyMozne.count() > 0) {
                probihaVolba = true;
                tlacitkaAktivni.append(p);
                ret = true;
            }
        } else {
            // další tlačítka cesty
            int pos = tlacitkaAktivni.count();
            QList<int> mozneCestyNove;
            mozneCestyNove.clear();
            for (int i : cestyMozne) {
                if (cesty->cesty.at(i)->tlacitka.at(pos) == p) {
                    //log(QString("cesty: možná cesta %1").arg(i), logging::LogLevel::Info);
                    mozneCestyNove.append(i);
                    ret = true;
                }
            }
            if (mozneCestyNove.count() > 0) {
                //log(QString("cesty: smazeme neplatne cesty ze seznamu"), logging::LogLevel::Info);
                cestyMozne = mozneCestyNove;
                tlacitkaAktivni.append(p);

                if (cestyMozne.count() == 1) {
                    //log(QString("cesty: jediná cesta %1 !!! - můžeme stavět").arg(cestyMozne.at(0)), logging::LogLevel::Info);
                    postavCestu(cestyMozne.at(0));
                    ret = true; // zapnout TZ u posledniho tlačítka
                }
            }
        }
        return ret;
    } else {
        // vytačení tlačítka
        if (!tlacitkaAktivni.isEmpty()) {
            if (tlacitkaAktivni.indexOf(p) >= 0) {
                p->r[TblokTC::TZ] = false;
                // zatím zrušíme celou cestu - neumíme VB
                ruseniVolbyCesty();
                return true;
            }
        }
        return false;
    }
}

void Tvoliciskupina::postavCestu(int i)
{
    // volící skupina se uvede do základního stavu
    //ruseniVolbyCesty();
    //mtbOutProbihaVolba.setValueBool(false);
    cestyMozne.clear();
    tlacitkaAktivni.clear();
    // cestu předáme do "dohledu"
    dohledCesty.postavCestu(i);
}

bool Tvoliciskupina::evaluate()
{
    mtbOutProbihaVolba.setValueBool(probihaVolba);

    if (mtbInRuseniVolby.value() && (!tlacitkaAktivni.isEmpty() || probihaVolba)) {
        // stisknuté tlačítko RV - rušení volby cesty
        log(QString("cesty: rušení volby"), logging::LogLevel::Info);
        ruseniVolbyCesty();
        return true;
    }
    if (rDCCVypadek || rDCCZkrat) {
        ruseniVolbyCesty();
    }
    return false;
}

Tvoliciskupina voliciskupina;
