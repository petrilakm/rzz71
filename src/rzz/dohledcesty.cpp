#include "dohledcesty.h"
#include "cesty.h"
#include "rzz/blokK.h"
#include "rzz/blokEMZ.h"
#include "voliciskupina.h"

TdohledCesty::TdohledCesty(){
    cestyPostavene.clear();
}

void TdohledCesty::postavCestu(int i) {
    struct cestaPodDohledem *nc = new cestaPodDohledem(); // nová cesta
    nc->num = i;
    nc->stav = scStavime;
    nc->vlakCelo = -1;
    nc->vlakKonec = -1;
    // přidáme na seznam platných cest
    cestyPostavene.append(nc);

    Tcesta *c;
    // najdeme správnou cestu
    c = cesty->cesty.at(i);

    // nastaví polohy prvkům v cestě
    //sepne VOP/VOM a simuluje výměnová automatická relé
    for (struct Tcesta::Tvyh v : c->polohy) {
        log(QString("dohled: aktivace %1 na výměne %2").arg((v.minus) ? "VOM" : "VOP").arg(v.pBlok->name), logging::LogLevel::Info);
        if (v.pBlok->typ == Tblok::btV) {
            // blok V se přestavý do správné polohy
            v.pBlok->r[TblokV::VOP] = !v.minus;
            v.pBlok->r[TblokV::VOM] =  v.minus;
        }
    }
}

// zhasne všechna tlačítka v cestě
void TdohledCesty::zhasniTlacitka(int i)
{
    Tcesta *cesta;
    cesta = cesty->cesty.at(i);
    if (cesta) {
        for (TblokTC *tlacitko : cesta->tlacitka) {
            //tlacitko->r[TblokTC::TZ] = false;
            tlacitko->r[TblokTC::PO] = false;
            tlacitko->r[TblokTC::TK] = false;
        }
    }
}

void TdohledCesty::t3C()
{

}

bool TdohledCesty::kontrolaCelistvostiCesty(Tcesta *c, bool cestaJizExistuje)
{
    bool stavOK = true;
    for(Tblok *blok : c->bloky) {
        if (blok->typ == Tblok::btS) {
            // blok S - kontrolujeme volnost a závěr
            if (blok->r[TblokS::J]) stavOK = false;
            if (cestaJizExistuje) {
                if (!blok->r[TblokS::Z]) stavOK = false; // zavěr musí být, když cesta již je postavená
            } else {
                if (blok->r[TblokS::Z]) stavOK = false; // závěr nesmí být, když cestu stavíme
            }
        }
        if (blok->typ == Tblok::btK) {
            // blok K - kontrolujeme výluková relé a volnost
            if (cestaJizExistuje) {
                // cesta je postavená, výluka musí nějaká být
                if (c->posun) {
                    // u posunu kontrolujeme výluky i K1, K2
                    if (!(blok->r[TblokK::X1]) || !(blok->r[TblokK::X2])) stavOK = false; // musí být aspoň 1 výluka
                } else {
                    // vlaková cesta se dívá jen na výluky
                    //if (!(blok->r[TblokK::X1]) || !(blok->r[TblokK::X2])) stavOK = false; // musí být aspoň 1 výluka
                    // kontrolujeme volnost (u VC)
                    if (blok->r[TblokK::J]) stavOK = false;
                }
            } else {
                // stavíme cestu, vyluky nesmí být
                if (c->posun) {
                    // u posunu kontrolujeme výluky i K1, K2
                    if ((blok->r[TblokK::X1] && !(blok->r[TblokK::K1])) || (blok->r[TblokK::X2] && !(blok->r[TblokK::K2]))) stavOK = false;
                } else {
                    // vlaková cesta se dívá jen na výluky
                    if (blok->r[TblokK::X1] || blok->r[TblokK::X2]) stavOK = false;
                    // kontrolujeme volnost (u VC)
                    if (blok->r[TblokK::J]) stavOK = false;
                }
            }
        }
    };
    return stavOK;
}

int TdohledCesty::urciNavest(int navZnak, TblokQ *nasledneNavestidlo)
{
    int nasl;
    if (!nasledneNavestidlo) {
        nasl = 0;
    } else {
        nasl = nasledneNavestidlo->navestniZnak;
    }
    // nasledující znaky (test, přivol., posun, zhasnuté) se berou jako stůj
    if ((nasl == 5) || (nasl == 8) || (nasl==9) || (nasl==10) || (nasl==13)) nasl = 0;
    // vyřeší povolenou jízdu
    if ((navZnak == 1) && (nasl == 0)) navZnak = 2;
    if ((navZnak == 1) && ((nasl == 4) || (nasl==6) || (nasl==7))) navZnak = 3;
    // vyřeší povolenou jízdu zníženou rychlostí
    if ((navZnak == 4) && (nasl == 0)) navZnak = 6;
    if ((navZnak == 4) && ((nasl == 4) || (nasl==6) || (nasl==7))) navZnak = 7;

    return navZnak;
}

QString TdohledCesty::stavCesty2QString(stavCesty sc)
{
    switch (sc) {
    case scStavime: return QString("scStavime"); break;
    case scZavery:  return QString("scZavery"); break;
    case scDN: return QString("scDN"); break;
    case scPrujezdVlaku: return QString("scPrujezdVlaku"); break;
    default: return QString("badState");
    }
    return QString("stavCesty2QString->nocase");
}

void TdohledCesty::evaluate()
{
    Tcesta *c;
    QList<struct cestaPodDohledem *> cestyNaSmazani;
    cestyNaSmazani.clear();
    TblokK *pBlokK;
    //Tblok *pBlok;
    bool stavOK;
    bool obv1 = false;
    bool obv2 = false;
    for (struct cestaPodDohledem *d : cestyPostavene) {
        c = cesty->cesty.at(d->num);

        // pro všechny stavy

        // když je nějaký úsek v cestě označen pro NUZ, cestu už nekontrolujeme.
        if (d->stav != scStavime) {
            for (Tblok *blok : c->bloky) {
                if (blok->typ == Tblok::btS) {
                    if (blok->r[TblokS::V]) {
                        cestyNaSmazani.append(d);
                    }
                }
            }
        }

        // vytáhnutí tlačítka ruší cestu, ale jen v počátečních fázích
        if (c->tlacitka[0]->mtbIns[TblokTC::mtbeIns::mtbInRuseni].value()) {
            // počáteční tlačítko je vytažené
            if ((d->stav == scStavime) || (d->stav == scZavery) || (d->stav == scDN)) {
                // zruší závěry - debug
                // doplnit aktivaci časového souboru
                if (d->stav != scStavime) { // ruší závěry jen postavené cesty, ne čekající
                    for (Tblok *b : c->bloky) {
                        if (b->typ == Tblok::btS) {
                            b->r[TblokS::Z] = false;
                            //b->r[TblokS::B] = false;
                        }
                    }
                }
                // cestu lze zrušit
                cestyNaSmazani.append(d);
            }
        }

        // chování podle stavu cesty
        switch (d->stav) {
        case scStavime: // kontrola výměn
            stavOK = true;
            // kontrola všech výhybek stavěných, zda už mají polohu
            foreach (struct Tcesta::Tvyh vymena, c->polohy) {
                // EMZ
                if (vymena.pBlok->typ == Tblok::btEMZ) {
                    if (vymena.pBlok->r[TblokEMZ::UK]) stavOK = false;
                    continue;
                }
                // Přestavník
                if (vymena.pBlok->typ == Tblok::btV) {
                    TblokV *dvojiceBlok = static_cast<TblokV*>(vymena.pBlok)->dvojceBlok;
                    if (! static_cast<TblokV*>(vymena.pBlok)->dvojceBlok) {
                        // pokud není dvojce kontroluje uvedenou výhybku
                        if ((!vymena.minus && !vymena.pBlok->r[TblokV::DP]) ||
                            ( vymena.minus && !vymena.pBlok->r[TblokV::DM])) {
                            stavOK = false;
                        }
                    } else {
                        // pokud je dvojice, tak kontroluje druhou půlku
                        if ((!vymena.minus && !dvojiceBlok->r[TblokV::DP]) ||
                            ( vymena.minus && !dvojiceBlok->r[TblokV::DM])) {
                            stavOK = false;
                        }
                    }
                }
            };
            if (stavOK) {
                log(QString("dohled: konec stavění výměn"), logging::LogLevel::Debug);
                // máme postaveno, můžeme zrušit výměnová automatická relé a VOP, VOM
                foreach (struct Tcesta::Tvyh vymena, c->polohy) {
                    if (vymena.pBlok->typ == Tblok::btV) {
                        log(QString("dohled: deaktivace VOP/VOM na výměně %1").arg(vymena.pBlok->name), logging::LogLevel::Debug);
                        vymena.pBlok->r[TblokV::VOP] = false;
                        vymena.pBlok->r[TblokV::VOM] = false;
                    };
                };
                // cesta se posune do dalšího stavu
                d->stav = scZavery;
            }
            break;
        case scZavery: // kontrola volnosti a padání závěrů úseků
            // kontrola volnosti JC
            stavOK = kontrolaCelistvostiCesty(c, false);
            if (stavOK) {
                // úseky jsou volné a nemají závěr z jiné cesty
                log(QString("dohled: provedeme závěr celé cesty číslo %1").arg(d->num), logging::LogLevel::Info);
                // vše v pořádku, provedeme závěr cesty
                for (int i = 0; i < c->bloky.count(); i++) {
                    Tblok *blok = c->bloky[i];
                    bool bPosledniBlok = (i == (c->bloky.count() - 1));
                    if (blok->typ == Tblok::btS) {
                        // poslední úsek pod závěr nedáváme
                        if (!bPosledniBlok) {
                            blok->r[TblokS::Z] = true; // aktivujeme závěrná relé
                            //blok->r[TblokS::B] = true;
                        }
                    }
                    // kolej bude kopírovat záver z předešlého úseku
                    if (blok->typ == Tblok::btK) {
                        pBlokK = static_cast<TblokK *>(blok);
                        // zjístíme, zda není již navázáno z druhé strany
                        if (pBlokK->predBlok1) {
                            pBlokK->predBlok2 = c->bloky[i-1];
                            pBlokK->r[TblokK::X2] = true;
                            if (c->posun) pBlokK->r[TblokK::K2] = true;
                        } else {
                            pBlokK->predBlok1 = c->bloky[i-1];
                            pBlokK->r[TblokK::X1] = true;
                            if (c->posun) pBlokK->r[TblokK::K1] = true;
                        }
                    }
                };
                log(QString("dohled: nastavení závěru odvratným výměnám"), logging::LogLevel::Debug);
                // nastaví závěrné useky odvratným výhybkám
                //for (int i =0; i < c->odvraty.count(); i++) {
                for (struct Tcesta::Tvyh_odv odv : c->odvraty) {
                    log(QString("dohled:  - výmena %1").arg(odv.pBlokVymena->name), logging::LogLevel::Info);
                    if (odv.pBlokVymena->typ == Tblok::btV) {
                        static_cast<TblokV*>(odv.pBlokVymena)->odvratneBloky.append(odv.pBlokUsek);
                    }
                    if (odv.pBlokVymena->typ == Tblok::btEMZ) {
                        static_cast<TblokEMZ*>(odv.pBlokVymena)->odvratneBloky.append(odv.pBlokUsek);
                    }
                }
                log(QString("dohled: zhasne tlačítka"), logging::LogLevel::Debug);
                zhasniTlacitka(c->num);
                d->stav = scDN;
            }
            break;
        case scDN:
            stavOK = kontrolaCelistvostiCesty(c,true);
            if (stavOK) {
                if (c->Navestidlo) {
                    c->Navestidlo->navestniZnak = urciNavest(c->navZnak, c->nasledneNavestidlo);
                    c->Navestidlo->r[TblokQ::N] = true;
                }
            } else {
                if (c->Navestidlo) {
                    c->Navestidlo->navestniZnak = 0;
                    c->Navestidlo->r[TblokQ::N] = false;
                }
                d->stav = scPrujezdVlaku;
            }
            break;
        case scPrujezdVlaku:
            if (d->vlakCelo >= 0) {
                if ((d->vlakCelo+1) < c->bloky.count()) {
                    // zjístí obsazení u čela vlaku
                    if (c->bloky[d->vlakCelo]->typ == Tblok::btS) {
                        obv1 = static_cast<TblokS*>(c->bloky[d->vlakCelo])->r[TblokS::J];
                    }
                    if (c->bloky[d->vlakCelo+1]->typ == Tblok::btS) {
                        obv2 = static_cast<TblokS*>(c->bloky[d->vlakCelo+1])->r[TblokS::J];
                    }
                    if (c->bloky[d->vlakCelo+1]->typ == Tblok::btK) {
                        obv2 = static_cast<TblokK*>(c->bloky[d->vlakCelo+1])->r[TblokK::J];
                    }
                    // vyhodnotí posun čela vlaku
                    if (obv1 && obv2) d->vlakEvidenceCelo = true;
                    if (!obv1 && obv2 && d->vlakEvidenceCelo) {
                        d->vlakEvidenceCelo = false;
                        d->vlakCelo++;
                    }
                    if (!obv1) d->vlakEvidenceCelo = false;
                }
            }
            break;
        case scRC: // bad states
            d->stav = scRC;
            break;
        }
    }
    // smaže cesty, co už nejsou cestami
    foreach (struct cestaPodDohledem *d, cestyNaSmazani) {
        // zhasnout tlačítka
        Tcesta *c = cesty->cesty[d->num];
        for (TblokTC *tc : c->tlacitka) {
            tc->r[TblokTC::PO] = false;
            tc->r[TblokTC::TK] = false;
        }


        // a smazat cestu
        cestyPostavene.removeOne(d);
        delete d;


    }
}

TdohledCesty dohledCesty;
