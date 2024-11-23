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
    nc->stav = scZvoleno;
    nc->pCesta = c;
    nc->vlakCelo = -1;
    nc->vlakKonec = -1;
    // přidáme na seznam platných cest
    cestyPostavene.append(nc);
    log(QString("dohled: přidání cesty číslo %1").arg(nc->num), logging::LogLevel::Commands);
}

// zhasne všechna tlačítka v cestě
void TdohledCesty::zhasniTlacitka(int i)
{
    Tcesta *cesta;
    cesta = cesty->cesty.at(i);
    if (cesta) {
        for (TblokTC *tlacitko : cesta->tlacitka) {
            tlacitko->r[TblokTC::TZ] = false;
            tlacitko->r[TblokTC::PO] = false;
            tlacitko->r[TblokTC::VA] = false;
        }
    }
}

void TdohledCesty::t3C()
{

}


/****************************************************************************/
// cestaKompletni = true -> závěry musí být všede, false -> závěr nesmí být nikde
bool TdohledCesty::cestaPodDohledem::kontrolaZavery(bool cestaKompletni)
{
    bool stavOK = true;
    upoZavery.clear();
    // pomocné proměné
    bool bBlockLast = false; // označuje poslední úsek cesty

    // závěry úseků a NUZ
    for(Tblok *blok : pCesta->bloky) {
        if (blok == pCesta->bloky.last()) bBlockLast = true; else bBlockLast = false;
        //if (blok == c->bloky.first()) bBlockFirst = true; else bBlockFirst = false;
        if (blok->typ == Tblok::btS) {
            // blok S - kontrolujeme závěr a NUZ
            if (blok->r[TblokS::rel::V]) {
                stavOK = false;
                this->upoZavery.append(QString("NUZ na %1").arg(blok->name));
            }
            if (cestaKompletni) {
                if (!bBlockLast) { // poslední blok závěr nemá
                    if (!blok->r[TblokS::rel::Z]) {
                        stavOK = false; // zavěr musí být, když cesta již je postavená
                        this->upoZavery.append(QString("ne zav %1").arg(blok->name));
                    }
                }
            } else {
                if (blok->r[TblokS::rel::Z]) {
                    stavOK = false; // závěr nesmí být, když cestu stavíme
                    this->upoZavery.append(QString("zav %1").arg(blok->name));
                }
            }
        }
        if (blok->typ == Tblok::btK) {
            // blok K - kontrolujeme výluková relé a volnost
            if (cestaKompletni) {
                // cesta je postavená, výluka musí nějaká být
                if (pCesta->posun) {
                    // u posunu kontrolujeme výluky i K1, K2
                    /*
                    if (!(blok->r[TblokK::X1]) || !(blok->r[TblokK::X2])) {
                        stavOK = false; // musí být aspoň 1 výluka
                        this->upoZavery.append(QString("ne výluky %1").arg(blok->name));
                    }
                    */
                } else {
                    // vlaková cesta se dívá jen na výluky
                    //if (!(blok->r[TblokK::X1]) || !(blok->r[TblokK::X2])) stavOK = false; // musí být aspoň 1 výluka
                }
            } else {
                // stavíme cestu, vyluky nesmí být
                if (pCesta->posun) {
                    // u posunu kontrolujeme výluky i K1, K2
                    if ((blok->r[TblokK::X1] && !(blok->r[TblokK::K1])) || (blok->r[TblokK::X2] && !(blok->r[TblokK::K2]))) {
                        stavOK = false;
                        this->upoZavery.append(QString("proti ces. %1").arg(blok->name));
                    }
                } else {
                    // vlaková cesta se dívá jen na výluky
                    if (blok->r[TblokK::X1] || blok->r[TblokK::X2]) {
                        stavOK = false;
                        this->upoZavery.append(QString("proti ces. %1").arg(blok->name));
                    }
                }
            }
        }
    };

    // závěry výhybek
    foreach (struct Tcesta::Tvyh vymena, pCesta->polohy) {
        // blokV
        if (vymena.pBlok->typ == Tblok::btV) {
            TblokV *vyh = static_cast<TblokV*>(vymena.pBlok);
            if (cestaKompletni) {
                // pro kontrolu návstidla musí být závěr
                if (!(vyh->r[TblokV::rel::Z])) {
                    stavOK = false;
                    this->upoZavery.append(QString("ne zav %1").arg(vymena.pBlok->name));
                }
            } else {
                // pro kontrolu stavění cestu závěr nesmí být
                if ((vyh->r[TblokV::rel::Z])) {
                    stavOK = false;
                    this->upoZavery.append(QString("zav %1").arg(vymena.pBlok->name));
                }
            }
        }
        // EMZ
        if (vymena.pBlok->typ == Tblok::btEMZ) {
            TblokEMZ *vyh = static_cast<TblokEMZ*>(vymena.pBlok);
            if (cestaKompletni) {
                // pro kontrolu návstidla musí být závěr
                if (!(vyh->r[TblokEMZ::rel::Z])) {
                    stavOK = false;
                    this->upoZavery.append(QString("ne zav %1").arg(vymena.pBlok->name));
                }
            } else {
                // pro kontrolu stavění cestu závěr nesmí být
                if ((vyh->r[TblokV::rel::Z])) {
                    stavOK = false;
                    this->upoZavery.append(QString("zav %1").arg(vymena.pBlok->name));
                }
            }
        }
    }

    return stavOK;
}

bool TdohledCesty::cestaPodDohledem::kontrolaVolnosti()
{
    bool stavOK = true;
    upoVolnosti.clear();

    // kontrola kolejových relé J
    for(Tblok *blok : pCesta->bloky) {
        if (blok->typ == Tblok::btS) {
            // blok S - kontrolujeme volnost
            if (blok->r[TblokS::J]) {
                stavOK = false;
                this->upoVolnosti.append(QString("obsaz. %1").arg(blok->name));
            }
        }
        if (blok->typ == Tblok::btK) {
            // blok K - kontrolujeme volnost
            if (!(pCesta->posun)) {
                // kontrolujeme volnost (jen u VC)
                if (blok->r[TblokK::J]) {
                    stavOK = false;
                    this->upoVolnosti.append(QString("obsaz %1").arg(blok->name));
                }
            }
        }
    };

    return stavOK;
}

bool TdohledCesty::cestaPodDohledem::kontrolaPolohVymen()
{
    bool stavOK = true;
    upoPolohy.clear();
    // kontrola pojížděných výhybek
    foreach (struct Tcesta::Tvyh vymena, pCesta->polohy) {
        // EMZ
        if (vymena.pBlok->typ == Tblok::btEMZ) {
            if (vymena.pBlok->r[TblokEMZ::UK]) {
                stavOK = false;
                upoPolohy += QString("EMZ ");
            }
        }
        // Přestavník
        if (vymena.pBlok->typ == Tblok::btV) {
            TblokV *vyh = static_cast<TblokV*>(vymena.pBlok);
            TblokV *dvojiceBlok = vyh->dvojceBlok;
            // vždy kontroluje uvedenou výhybku
            if (!(vymena.minus) && (!(vyh->r[TblokV::rel::DP]))) {
                stavOK = false;
                upoPolohy += QString("%1+ ").arg(vymena.pBlok->name);
            }
            if ( (vymena.minus) && (!(vyh->r[TblokV::rel::DM]))) {
                stavOK = false;
                upoPolohy += QString("%1- ").arg(vymena.pBlok->name);
            }
            if (dvojiceBlok != nullptr) {
                // pokud je dvojice, tak kontroluje i druhou půlku
                if (!(vymena.minus) && (!(dvojiceBlok->r[TblokV::rel::DP]))) {
                    stavOK = false;
                    upoPolohy += QString("%1+ ").arg(dvojiceBlok->name);
                }
                if ( (vymena.minus) && (!(dvojiceBlok->r[TblokV::rel::DM]))) {
                    stavOK = false;
                    upoPolohy += QString("%1- ").arg(dvojiceBlok->name);
                }
            }
        }
    };

    // kontrola poloh odvratů
    foreach (struct Tcesta::Tvyh_odv odv, pCesta->odvraty) {
        Tblok *vymena = (odv.pBlokVymena);
        // EMZ
        if (vymena->typ == Tblok::btEMZ) {
            if (vymena->r[TblokEMZ::rel::UK]) {
                stavOK = false;
                upoPolohy += QString("EMZ ");
            }
        }
        // Přestavník
        if (vymena->typ == Tblok::btV) {
            TblokV *vyh = static_cast<TblokV*>(vymena);
            TblokV *dvojiceBlok = static_cast<TblokV*>(vymena)->dvojceBlok;
            if (!(odv.minus) && (!(vyh->r[TblokV::rel::DP]))) {
                stavOK = false;
                upoPolohy += QString("od%1+ ").arg(vyh->name);
            }
            if ( (odv.minus) && (!(vyh->r[TblokV::rel::DM]))) {
                stavOK = false;
                upoPolohy += QString("od%1- ").arg(vyh->name);
            }
            if (dvojiceBlok != nullptr) {
                // pokud je dvojice, tak kontroluje i druhou půlku
                if (!(odv.minus) && (!(dvojiceBlok->r[TblokV::rel::DP]))) {
                    stavOK = false;
                    upoPolohy += QString("od%1+ ").arg(dvojiceBlok->name);
                }
                if ( (odv.minus) && (!(dvojiceBlok->r[TblokV::rel::DM]))) {
                    stavOK = false;
                    upoPolohy += QString("od%1- ").arg(dvojiceBlok->name);
                }
            }
        }
    };

    return stavOK;
}

bool TdohledCesty::cestaPodDohledem::kontrolaNUZ()
{
    bool stavOK = true;

    if ((this->stav != scStavime) && (this->stav != scZvoleno)) {
        for (Tblok *blok : this->pCesta->bloky) {
            if (blok->typ == Tblok::btS) {
                if (blok->r[TblokS::rel::V]) {
                    stavOK = false;
                }
            }
        }
    }

    return stavOK;
}

// kontrola, zda je možné cestu postavit - zapojení výměnových ovládacích relé
// (kniha s.48?)
// pokud je výhýbka v cestě a má VOP/VOM na opačnou stranu, tak nestavíme
bool TdohledCesty::cestaPodDohledem::kontrolaVOPVOM()
{
    bool stavOK = true;
    upoVOPVOM.clear();

    for(struct Tcesta::Tvyh vc : pCesta->polohy) {
        if (vc.pBlok->typ == Tblok::btV) {
            TblokV *v = static_cast<TblokV *>(vc.pBlok);
            if (vc.minus) {
                if (v->r[TblokV::rel::VOP]) stavOK = false;
            } else {
                if (v->r[TblokV::rel::VOM]) stavOK = false;
            }
        }
    }
    // to samé pro odvraty
    for(struct Tcesta::Tvyh_odv vc : pCesta->odvraty) {
        if (vc.pBlokVymena->typ == Tblok::btV) {
            TblokV *v = static_cast<TblokV *>(vc.pBlokVymena);
            if (vc.minus) {
                if (v->r[TblokV::rel::VOP]) stavOK = false;
            } else {
                if (v->r[TblokV::rel::VOM]) stavOK = false;
            }
        }
    }

    return stavOK;
}

void TdohledCesty::cestaPodDohledem::povelVAzapnout()
{
    // sepne VOP/VOM
    for (struct Tcesta::Tvyh v : pCesta->polohy) {
        log(QString("dohled: aktivace %1 na výměne %2")
                .arg((v.minus) ? "VOM" : "VOP", v.pBlok->name),
            logging::LogLevel::Commands);
        if (v.pBlok->typ == Tblok::btV) {
            // blok V se přestavý do správné polohy
            v.pBlok->r[TblokV::VOP] = !v.minus;
            v.pBlok->r[TblokV::VOM] =  v.minus;
        }
    }
    // sepne výměnová automatická relé
    for(int i = 0; i < pCesta->tlacitka.count(); i++) {
        TblokTC *t = pCesta->tlacitka[i];
        if (i == 0) t->r[TblokTC::PO] = true; // první tlačítko stále bliká
        t->r[TblokTC::VA] = true; // další tlačítka v cestě svítí trvale - výměnová automatika v činnosti
    }
}

void TdohledCesty::cestaPodDohledem::povelVAvypnout()
{
    // vypne VOP/VOM
    for (struct Tcesta::Tvyh v : pCesta->polohy) {
        log(QString("dohled: deaktivace VOP/VOM na výměne %1")
                .arg(v.pBlok->name),
            logging::LogLevel::Commands);
        if (v.pBlok->typ == Tblok::btV) {
            v.pBlok->r[TblokV::rel::VOP] = false;
            v.pBlok->r[TblokV::rel::VOM] = false;
        }
    }
    // vypne výměnová automatická relé
    for(int i = 0; i < pCesta->tlacitka.count(); i++) {
        TblokTC *t = pCesta->tlacitka[i];
        t->r[TblokTC::rel::VA] =  false; // výměnová automatika vypnout
    }
}

void TdohledCesty::cestaPodDohledem::povelPOvypnout()
{
    // vypne protiopakovací relé a tlačítková relé
    for(int i = 0; i < pCesta->tlacitka.count(); i++) {
        TblokTC *t = pCesta->tlacitka[i];
        t->r[TblokTC::rel::TZ] =  false;
        t->r[TblokTC::rel::PO] =  false;
    }
}

/****************************************************************************/
// *c - cesta co chceme zkontrolovat
// cestaJizExistuje - rozlišuje zda chceme cestu postavit, nebo kontrolujeme podmínky DN
bool TdohledCesty::cestaPodDohledem::kontrolaCelistvostiCesty()
{
    // předpokládáme, že je vše ok, jakákoliv chyba to ale změní
    bool stavOK = true;
    // nulování UPO - sestaví se znova kontrolou
    this->upo.clear();

    stavOK &= kontrolaPolohVymen();
    stavOK &= kontrolaZavery(true);
    stavOK &= kontrolaVolnosti();

    // spojí všechna UPO
    if (upoPolohy.length() > 0) {
        this->upo.append(upoPolohy);
    }
    if (upoZavery.length() > 0) {
        this->upo.append(upoZavery);
    }
    if (upoVolnosti.length() > 0) {
        this->upo.append(upoVolnosti);
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
    case scZvoleno: return QString("Zvoleno"); break;
    case scStavime: return QString("Stavime"); break;
    case scZavery:  return QString("Zavery"); break;
    case scKontrolaDN: return QString("KontrolaDN"); break;
    case scDN: return QString("DN"); break;
    case scPrujezdVlaku: return QString("Prujezd"); break;
    case scProjeto: return QString("Projeto"); break;
    case scRC: return QString("RC"); break;
    case scZbytek: return QString("Zbytek"); break;
    default: return QString("badState");
    }
    return QString("stavCesty2QString->nocase");
}

void TdohledCesty::evaluate()
{
    Tcesta *c;
    QList<cestaPodDohledem *> cestyNaSmazani;
    cestyNaSmazani.clear();
    TblokK *pBlokK;
    TblokV *pBlokV;
    TblokEMZ *pBlokEMZ;
    Tblok *pBlok;
    bool stavOK;
    bool obs1 = false;
    bool obs2 = false;
    for (cestaPodDohledem *d : cestyPostavene) {
        c = d->pCesta;
        d->upo.clear();
        // pro všechny stavy

        // když je nějaký úsek v cestě označen pro NUZ, cestu označíme jako nefunkční.
        if (!(d->kontrolaNUZ())) {
            d->stav = scZbytek;
        }

        // vytáhnutí tlačítka ruší cestu, ale jen ve správných fázích
        if (c->tlacitka[0]->mtbIns[TblokTC::mtbeIns::mtbInRuseni].value()) {
            // počáteční tlačítko je vytažené
            if ((d->stav == scZavery) || (d->stav == scKontrolaDN) || (d->stav == scDN)) {
                // zruší závěry - debug
                log(QString("dohled: rušení počátkem cesty číslo %1").arg(d->num), logging::LogLevel::Commands);
                // doplnit aktivaci časového souboru
                for (Tblok *b : c->bloky) {
                    if (b->typ == Tblok::btS) {
                        b->r[TblokS::Z] = false;
                        //b->r[TblokS::B] = false;
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
                if (d->kontrolaCelistvostiCesty()) {
                    // obnovíme cestu
                    d->stav = scKontrolaDN;
                    log(QString("dohled: obnova DN cesty číslo %1").arg(d->num), logging::LogLevel::Commands);
                }
            }
        }

        // chování podle stavu cesty
        switch (d->stav) {
        case scZvoleno:
            stavOK = true;
            // pokud má některá s výhýbek VOP/VOM na opačnou stranu, tak nestavíme
            stavOK &= d->kontrolaVOPVOM();
            // kontrola závěrů
            stavOK &= d->kontrolaZavery(false);
            // sestaví UPO
            d->upo.append(d->upoZavery);
            d->upo.append(d->upoVOPVOM);
            // povel pro zrušení od volící skupiny
            if (voliciskupina.mtbInRuseniVolby.value()) {
                cestyNaSmazani.append(d);
                stavOK = false;
            }
            // povel pro stavění
            if (stavOK) {
                // sepne VOP/VOM a výměnová automatická relé
                d->povelVAzapnout();
                // posun do dalšího stavu
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scStavime)), logging::LogLevel::Commands);
                d->stav = scStavime;

            }

            break;
        case scStavime: // kontrola výměn
            // kontrola všech výhybek v cestě i odvratů, zda už mají polohu
            stavOK = d->kontrolaPolohVymen();
            stavOK &= d->kontrolaZavery(false);
            d->upo.append(d->upoZavery);
            d->upo.append(d->upoPolohy);

            // povel pro zrušení od volící skupiny
            if (voliciskupina.mtbInRuseniVolby.value()) {
                cestyNaSmazani.append(d);
                d->povelVAvypnout(); // nezapomenou vypnout VA !
                stavOK = false;
            }

            if (stavOK) {
                // cesta se posune do dalšího stavu
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scZavery)), logging::LogLevel::Commands);
                d->stav = scZavery;
            }

            break;
        case scZavery: // kontrola volnosti a padání závěrů úseků (kontrola souhlasů)
            stavOK = true;
            stavOK &= d->kontrolaPolohVymen();
            stavOK &= d->kontrolaZavery(false);
            d->upo.append(d->upoZavery);
            d->upo.append(d->upoPolohy);
            if (stavOK) {
                // úseky jsou volné a nemají závěr z jiné cesty
                log(QString("dohled: provedeme závěr celé cesty číslo %1").arg(d->num), logging::LogLevel::Commands);
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


                log(QString("dohled: nastavení závěru odvratným výměnám"), logging::LogLevel::Commands);
                // nastaví závěrné useky odvratným výhybkám
                for (struct Tcesta::Tvyh_odv odv : c->odvraty) {
                    log(QString("dohled:  - odvrat výmena %1").arg(odv.pBlokVymena->name), logging::LogLevel::Commands);
                    // zámky
                    if (odv.pBlokVymena->typ == Tblok::btEMZ) {
                        pBlokEMZ = static_cast<TblokEMZ*>(odv.pBlokVymena);
                        pBlokEMZ->odvratneBloky.append(odv.pBlokUsek);
                    }
                    // výhybky
                    if (odv.pBlokVymena->typ == Tblok::btV) {
                        pBlokV = static_cast<TblokV*>(odv.pBlokVymena);
                        pBlokV->odvratneBloky.append(odv.pBlokUsek);
                    }
                }

                d->povelVAvypnout();
                /*
                // zrušíme výměnová automatická relé
                for(int i = 0; i < c->tlacitka.count(); i++) {
                    TblokTC *t = c->tlacitka[i];
                    t->r[TblokTC::VA] = false; // zrušeno udělením závěrů useků
                }

                log(QString("dohled: konec stavění výměn"), logging::LogLevel::Debug);
                // máme postaveno, můžeme zrušit výměnová automatická relé a VOP, VOM
                foreach (struct Tcesta::Tvyh vymena, c->polohy) {
                    if (vymena.pBlok->typ == Tblok::btV) {
                        log(QString("dohled: deaktivace VOP/VOM na výměně %1").arg(vymena.pBlok->name), logging::LogLevel::Commands);
                        vymena.pBlok->r[TblokV::VOP] = false;
                        vymena.pBlok->r[TblokV::VOM] = false;
                    };
                };
    */
                log(QString("dohled: zhasne tlačítka cesty"), logging::LogLevel::Commands);
                //zhasniTlacitka(c->num);
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scKontrolaDN)), logging::LogLevel::Commands);
                d->stav = scKontrolaDN;
            }
            break;
        case scKontrolaDN:
            stavOK = d->kontrolaCelistvostiCesty();
            if (stavOK) {
                if (c->Navestidlo) {
                    c->Navestidlo->navestniZnak = urciNavest(c->navZnak, c->nasledneNavestidlo);
                    c->Navestidlo->r[TblokQ::N] = true;
                    d->vlakCelo = -1;
                    d->vlakKonec = -1;
                    d->vlakEvidenceCelo = false;
                    d->vlakEvidenceKonec = false;
                    d->povelPOvypnout();
                    log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scDN)), logging::LogLevel::Commands);
                    d->stav = scDN;
                }
            }
            break;
        case scDN:
            stavOK = d->kontrolaCelistvostiCesty();
            if (stavOK) {
                if (c->Navestidlo) {
                    if (c->Navestidlo->r[TblokQ::N]) {
                        // DN - urči návěst
                        c->Navestidlo->navestniZnak = urciNavest(c->navZnak, c->nasledneNavestidlo);
                    } else {
                        // není DN
                        log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scPrujezdVlaku)), logging::LogLevel::Commands);
                        d->stav = scPrujezdVlaku;
                    }
                }
            } else {
                if (c->Navestidlo) {
                    c->Navestidlo->navestniZnak = 0;
                    c->Navestidlo->r[TblokQ::N] = false;
                }
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scPrujezdVlaku)), logging::LogLevel::Commands);
                d->stav = scPrujezdVlaku;
            }
            break;
        case scPrujezdVlaku:
            // ToDo: dodělat logiku čela a koncevlaku
            // pokud nejsme na konci, hýbene s čelem vlaku
            if (d->vlakCelo < c->bloky.count()) {
                obs1 = c->zjistiObsazeni(d->vlakCelo);
                obs2 = c->zjistiObsazeni(d->vlakCelo+1);
                if (obs1 && obs2) {
                    if (d->vlakCelo == -1) {
                        d->vlakEvidenceCelo = true;
                    }
                    d->vlakCelo++;
                }
            }

            // inicializace konce při průjezdu
            if (d->vlakCelo == 0) {
                d->vlakKonec = 0;
            }

            // pokud nejsme na konci, hýbene s koncem vlaku
            if (d->vlakKonec < c->bloky.count()) {
                obs1 = c->zjistiObsazeni(d->vlakKonec);
                obs2 = c->zjistiObsazeni(d->vlakKonec+1);
                if (!obs1 && obs2) {
                    if (cfgVybav) {
                        // postupné rušení závěrů
                        c->uvolniZaver(d->vlakKonec);
                    }
                    d->vlakKonec++;
                }
            }

            if ((d->vlakCelo == c->bloky.count()) && ((d->vlakKonec+1) == c->bloky.count())) {
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scProjeto)), logging::LogLevel::Commands);
                d->stav = scProjeto;
            }
            break;
        case scProjeto:
            if (cfgVybav) {
                log(QString("dohled: cesta č. %1 kompletně vybavena").arg(d->num), logging::LogLevel::Commands);
                cestyNaSmazani.append(d);
            } else {
                // ToDo: aktivovat tlačítko na ruční RC (na daném zhlavý)
            }
            break;
        case scRC: // RC
            d->stav = scRC;
            break;
        case scZbytek: // bad state
            d->upo.append("nekompletni");
            if (d->kontrolaZavery(false)) {
                log(QString("dohled: cesta č. %1 zrušena po kouskách").arg(d->num), logging::LogLevel::Commands);
                cestyNaSmazani.append(d);
            }
            break;
        }
    }
    // smaže cesty, co už nejsou cestami
    foreach (cestaPodDohledem *d, cestyNaSmazani) {

        Tcesta *c = cesty->cesty[d->num];
        if ((d->stav == scZvoleno) || (d->stav == scStavime)) {
            for (TblokTC *tc : c->tlacitka) {
                // zhasnout tlačítka
                tc->r[TblokTC::TZ] = false;
                tc->r[TblokTC::PO] = false;
                tc->r[TblokTC::VA] = false;
            }
        }
        if (d->stav == scStavime) {
            d->povelVAvypnout();
        }
        // a smazat cestu
        cestyPostavene.removeOne(d);
        delete d;
    }
}

TdohledCesty dohledCesty;
