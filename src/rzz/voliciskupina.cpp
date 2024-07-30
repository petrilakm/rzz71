#include "voliciskupina.h"
#include "dohledcesty.h"

Tvoliciskupina::Tvoliciskupina()
{
//    bProbihaVolba = false;
}

void Tvoliciskupina::ruseniVolbyCesty()
{
    cestyMozne.clear();
    foreach (TblokTC * tl, tlacitkaAktivni) {
        tl->r[TblokTC::TZ] = false;
        //tl->r[TblokTC::TK] = false;
    }
    tlacitkaAktivni.clear();
    mtbOutProbihaVolba.setValueBool(false);
}

bool Tvoliciskupina::vstupZmena(TblokTC *p, bool state)
{
    bool ret = false;
    if (state) {
        //log(QString("cesty: změna tlačítka %1").arg(p->name), logging::LogLevel::Info);
        if (tlacitkaAktivni.isEmpty()) {
            // první tlačítko cesty
            cestyMozne.clear();
            for (Tcesta *i : cesty->cesty) {
                //log(QString("cesty: hledání %1 == %2").arg(p->name).arg(i->tlacitka.at(0)->name), logging::LogLevel::Info);
                if (i->tlacitka[0] == p) {
                    cestyMozne.append(i->num);
                    //log(QString("cesty: nalezeno %1").arg(i->num), logging::LogLevel::Info);
                }
            }
            if (cestyMozne.count() > 0) {
                mtbOutProbihaVolba.setValueBool(true);
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
                } else {
                    //log(QString("cesty: odstraněná cesta %1").arg(i), logging::LogLevel::Info);
                    //cestyMozne.remove(i);
                }
            }
            if (mozneCestyNove.count() > 0) {
                //log(QString("cesty: smazeme neplatne cesty ze seznamu"), logging::LogLevel::Info);
                cestyMozne = mozneCestyNove;
                tlacitkaAktivni.append(p);

                if (cestyMozne.count() == 1) {
                    //log(QString("cesty: jediná cesta %1 !!! - můžeme stavět").arg(cestyMozne.at(0)), logging::LogLevel::Info);
                    postavCestu(cestyMozne.at(0));
                    ret = false; // nezapinat TZ u posledniho tlačítka
                }
            }
        }
        return ret;
    } else {
        if (!tlacitkaAktivni.isEmpty()) {
            if (tlacitkaAktivni.indexOf(p) >= 0) {
                p->r[TblokTC::TZ] = false;
                // zatím zrušíme celou cestu - neumíme VB
                ruseniVolbyCesty();
            }
        }
        return false;
    }
}

void Tvoliciskupina::postavCestu(int i)
{
    Tcesta *c = cesty->cesty[i];

    for(int i = 0; i < c->tlacitka.count(); i++) {
        if (i==0) c->tlacitka[i]->r[TblokTC::TK] = true; // první tlačítko svítí trvale
        c->tlacitka[i]->r[TblokTC::PO] = true; // zapne protiopakovací relé
    }

    // volící skupina se uvede do základního stavu
    ruseniVolbyCesty();
    tlacitkaAktivni.clear();
    // cestu předáme do "dohledu"
    dohledCesty.postavCestu(i);

}

bool Tvoliciskupina::evaluate()
{
    if (mtbInRuseniVolby.value() && !tlacitkaAktivni.isEmpty()) {
        // stisknuté tlačítko RV - rušení volby cesty
        log(QString("cesty: rušení volby"), logging::LogLevel::Info);
        ruseniVolbyCesty();
        return true;
    }
    return false;
}

Tvoliciskupina voliciskupina;
