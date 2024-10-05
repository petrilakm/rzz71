#include "dohledcesty.h"
#include "cesty.h"
#include "rzz/blokK.h"
#include "rzz/blokEMZ.h"
#include "voliciskupina.h"

TdohledCesty::TdohledCesty(){
    cestyPostavene.clear();
}

void TdohledCesty::postavCestu(int i) {
    cestaPodDohledem *nc = new cestaPodDohledem(); // nová cesta
    Tcesta *c;
    // najdeme správnou cestu
    c = cesty->cesty.at(i);
    nc->num = i;
    nc->stav = scStavime;
    nc->pCesta = c;
    nc->vlakCelo = -1;
    nc->vlakKonec = -1;
    // přidáme na seznam platných cest
    cestyPostavene.append(nc);

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

/****************************************************************************/
// *c - cesta co chceme zkontrolovat
// cestaJizExistuje - rozlišuje zda chceme cestu postavit, nebo kontrolujeme podmínky DN
bool TdohledCesty::cestaPodDohledem::kontrolaCelistvostiCesty(bool cestaJizExistuje)
{
    // předpokládáme, že je vše ok, jakákoliv chyba to ale změní
    bool stavOK = true;
    // pomocné proměné
    bool bBlockLast = false; // označuje poslední úsek cesty
    bool bBlockFirst = false; // označuje první úsek cesty
    // nulování UPO - sestaví se zmova kontrolou
    this->upo.clear();
    // najdeme si cestu
    Tcesta *c = cesty->cesty.at(this->num);

    // kontrola bloků v cestě
    for(Tblok *blok : c->bloky) {
        if (blok == c->bloky.last()) bBlockLast = true; else bBlockLast = false;
        if (blok == c->bloky.first()) bBlockFirst = true; else bBlockFirst = false;
        if (blok->typ == Tblok::btS) {
            // blok S - kontrolujeme volnost a závěr
            if (blok->r[TblokS::J]) {
                stavOK = false;
                this->upo.append(QString("obsaz. %1").arg(blok->name));
            }
            if (cestaJizExistuje) {
                if (!bBlockLast) { // poslední blok závěr nemá
                    if (!blok->r[TblokS::Z]) {
                        stavOK = false; // zavěr musí být, když cesta již je postavená
                        this->upo.append(QString("ne záv. %1").arg(blok->name));
                    }
                }
            } else {
                if (blok->r[TblokS::Z]) {
                    stavOK = false; // závěr nesmí být, když cestu stavíme
                    this->upo.append(QString("záv. %1").arg(blok->name));
                }
            }
        }
        if (blok->typ == Tblok::btK) {
            // blok K - kontrolujeme výluková relé a volnost
            if (cestaJizExistuje) {
                // cesta je postavená, výluka musí nějaká být
                if (c->posun) {
                    // u posunu kontrolujeme výluky i K1, K2
                    if (!(blok->r[TblokK::X1]) || !(blok->r[TblokK::X2])) {
                        stavOK = false; // musí být aspoň 1 výluka
                        this->upo.append(QString("ne výluky %1").arg(blok->name));
                    }
                } else {
                    // vlaková cesta se dívá jen na výluky
                    //if (!(blok->r[TblokK::X1]) || !(blok->r[TblokK::X2])) stavOK = false; // musí být aspoň 1 výluka
                    // kontrolujeme volnost (u VC)
                    if (blok->r[TblokK::J]) {
                        stavOK = false;
                        this->upo.append(QString("obsaz. %1").arg(blok->name));
                    }
                }
            } else {
                // stavíme cestu, vyluky nesmí být
                if (c->posun) {
                    // u posunu kontrolujeme výluky i K1, K2
                    if ((blok->r[TblokK::X1] && !(blok->r[TblokK::K1])) || (blok->r[TblokK::X2] && !(blok->r[TblokK::K2]))) {
                        stavOK = false;
                        this->upo.append(QString("proti ces. %1").arg(blok->name));
                    }
                } else {
                    // vlaková cesta se dívá jen na výluky
                    if (blok->r[TblokK::X1] || blok->r[TblokK::X2]) {
                        stavOK = false;
                        this->upo.append(QString("proti ces. %1").arg(blok->name));
                    }
                    // kontrolujeme volnost (u VC)
                    if (blok->r[TblokK::J]) {
                        stavOK = false;
                        this->upo.append(QString("obsaz %1").arg(blok->name));
                    }
                }
            }
        }
    };
    // kontrola poloh výměn
    for(struct Tcesta::Tvyh vyh: c->polohy) {
        // blokV - musí být dohled polohy
        if (vyh.pBlok->typ == Tblok::btV) {
            if (!(vyh.minus) && (!(static_cast<TblokV*>(vyh.pBlok)->DP))) {
                stavOK = false;
                this->upo.append(QString("ne dohl+ %1").arg(vyh.pBlok->name));
            }
            if ( (vyh.minus) && (!(static_cast<TblokV*>(vyh.pBlok)->DM))) {
                stavOK = false;
                this->upo.append(QString("ne dohl- %1").arg(vyh.pBlok->name));
            }
            if (cestaJizExistuje) {
                // pro kontrolu návstidla musí být závěr
                if (!(static_cast<TblokV*>(vyh.pBlok)->r[TblokV::rel::Z])) {
                    stavOK = false;
                    this->upo.append(QString("ne záv. %1").arg(vyh.pBlok->name));
                }
            }
        }
        // blokEMZ - nesmí být uvolněn klíč
        if (vyh.pBlok->typ == Tblok::btEMZ) {
            if ((static_cast<TblokEMZ*>(vyh.pBlok)->r[TblokEMZ::rel::UK])) {
                stavOK = false;
                this->upo.append(QString("zámek %1").arg(vyh.pBlok->name));
            }
        }
    }
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
    case scKontrolaDN: return QString("scKontrolaPodminekProDN"); break;
    case scDN: return QString("scDN"); break;
    case scPrujezdVlaku: return QString("scPrujezdVlaku"); break;
    case scRC: return QString("scRC"); break;
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

        // stisknutí tlačítka může obnovit DN, když je správná situace
        if (c->tlacitka[0]->mtbIns[TblokTC::mtbeIns::mtbInVolba].value()) {
            // počáteční tlačítko je stisknuté
            if (d->stav == scPrujezdVlaku) {
                // jsme v režimu kontroly podmének pro DN
                if (d->kontrolaCelistvostiCesty(true)) {
                    // obnovíme cestu
                    d->stav = scKontrolaDN;
                }
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
            stavOK = d->kontrolaCelistvostiCesty(false);
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
                log(QString("dohled: zhasne tlačítka cesty"), logging::LogLevel::Debug);
                zhasniTlacitka(c->num);
                d->stav = scKontrolaDN;
            }
            break;
        case scKontrolaDN:
            stavOK = d->kontrolaCelistvostiCesty(true);
            if (stavOK) {
                if (c->Navestidlo) {
                    c->Navestidlo->navestniZnak = urciNavest(c->navZnak, c->nasledneNavestidlo);
                    c->Navestidlo->r[TblokQ::N] = true;
                    d->vlakCelo = -1;
                    d->vlakKonec = -1;
                    d->vlakEvidenceCelo = false;
                    d->vlakEvidenceKonec = false;
                    d->stav = scDN;
                }
            }
            break;
        case scDN:
            stavOK = d->kontrolaCelistvostiCesty(true);
            if (stavOK) {
                if (c->Navestidlo) {
                    if (c->Navestidlo->r[TblokQ::N]) {
                        // DN - urči návěst
                        c->Navestidlo->navestniZnak = urciNavest(c->navZnak, c->nasledneNavestidlo);
                    } else {
                        // není DN
                        d->stav = scPrujezdVlaku;
                    }
                }
            } else {
                if (c->Navestidlo) {
                    c->Navestidlo->navestniZnak = 0;
                    c->Navestidlo->r[TblokQ::N] = false;
                }
                d->stav = scPrujezdVlaku;
                d->vlakCelo=0;
            }
            break;
        case scPrujezdVlaku:
            // ToDo: dodělat logiku čela a koncevlaku
            // teď je to uplně blbě, čelo se pohybuje spíš jako konec
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
