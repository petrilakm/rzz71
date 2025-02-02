#include "dohledcesty.h"
#include "cesty.h"
#include "rzz/blokK.h"
#include "rzz/blokEMZ.h"
#include "rzz/blokKU.h"
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
    nc->vlakEV = false;
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

/****************************************************************************/
// cestaKompletni = true -> závěry musí být všede, false -> závěr nesmí být nikde
bool TdohledCesty::cestaPodDohledem::kontrolaZavery(bool cestaKompletni)
{
    bool stavOK = true;
    QStringList upoL; // upo - lokální
    // pomocné proměné
    bool bBlockLast = false; // označuje poslední úsek cesty

    // závěry úseků a NUZ
    //for(Tblok *blok : pCesta->bloky) {
    Tblok *blok;
    for (int i = 1; i < pCesta->bloky.count(); i++) {
        blok = pCesta->bloky[i];
        if (blok == pCesta->bloky.last()) bBlockLast = true; else bBlockLast = false;
        //if (blok == c->bloky.first()) bBlockFirst = true; else bBlockFirst = false;
        if (blok->typ == Tblok::btS) {
            // blok S - kontrolujeme závěr a NUZ
            if (blok->r[TblokS::rel::V]) {
                stavOK = false;
                upoL.append(QString("NUZ %1,").arg(blok->name));
            }
            if (cestaKompletni) {
                if (!bBlockLast) { // poslední blok závěr nemá
                    if (!blok->r[TblokS::rel::Z]) {
                        stavOK = false; // zavěr musí být, když cesta již je postavená
                        upoL.append(QString("ne zav %1,").arg(blok->name));
                    }
                }
            } else {
                if (blok->r[TblokS::rel::Z]) {
                    stavOK = false; // závěr nesmí být, když cestu stavíme
                    upoL.append(QString("zav %1,").arg(blok->name));
                }
            }
        }
        if (blok->typ == Tblok::btK) {
            // blok K - kontrolujeme výluková relé a volnost
            if (cestaKompletni) {
                // cesta je postavená, výluka musí nějaká být
                if (pCesta->posun) {
                    // u posunu kontrolujeme výluky i K1, K2
                    if (!(blok->r[TblokK::X1]) && !(blok->r[TblokK::X2])) {
                        stavOK = false; // musí být aspoň 1 výluka
                        upoL.append(QString("ne vyl %1").arg(blok->name));
                    }

                } else {
                    // vlaková cesta se dívá jen na výluky
                    if (!(blok->r[TblokK::X1]) && !(blok->r[TblokK::X2])) {
                        stavOK = false; // musí být aspoň 1 výluka
                        upoL.append(QString("ne vyl %1").arg(blok->name));
                    }
                }
            } else {
                // stavíme cestu, vyluky nesmí být
                if (pCesta->posun) {
                    // u posunu kontrolujeme výluky i konce posunové K1, K2
                    if ((blok->r[TblokK::X1] && !(blok->r[TblokK::K1])) || (blok->r[TblokK::X2] && !(blok->r[TblokK::K2]))) {
                        stavOK = false;
                        upoL.append(QString("proti ces. %1,").arg(blok->name));
                    }
                } else {
                    // vlaková cesta se dívá jen na výluky
                    if (blok->r[TblokK::X1] || blok->r[TblokK::X2]) {
                        stavOK = false;
                        upoL.append(QString("proti ces. %1,").arg(blok->name));
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
                    upoL.append(QString("ne zav %1").arg(vymena.pBlok->name));
                }
            } else {
                // pro kontrolu stavění cesty závěr nekontrolujeme
            }
        }
        // EMZ
        if (vymena.pBlok->typ == Tblok::btEMZ) {
            TblokEMZ *vyh = static_cast<TblokEMZ*>(vymena.pBlok);
            if (cestaKompletni) {
                // pro kontrolu návstidla musí být závěr
                if (!(vyh->r[TblokEMZ::rel::Z])) {
                    stavOK = false;
                    upoL.append(QString("ne zav %1").arg(vymena.pBlok->name));
                }
            } else {
                // pro kontrolu stavění cesty závěr nekontrolujeme
            }
        }
    }
    // sestavý UPO
    upoZavery.clear();
    upoZavery = upoL.join(',');
    return stavOK;
}

bool TdohledCesty::cestaPodDohledem::kontrolaVolnosti()
{
    bool stavOK = true;
    QStringList upoL;
    upoL.clear();

    // kontrola kolejových relé J
    //for(Tblok *blok : pCesta->bloky) {
    Tblok *blok;
    for (int i = 1; i < pCesta->bloky.count(); i++) {
        blok = pCesta->bloky[i];
        // poslední blok? - pro posun nekontrolujeme
        if ((i+1) == pCesta->bloky.count()) {
            if (pCesta->posun) continue;
        }
        // blok S - kontrolujeme volnost
        if (blok->typ == Tblok::btS) {
            if (blok->r[TblokS::rel::J]) {
                stavOK = false;
                upoL.append(QString("obs %1").arg(blok->name));
            }
        }
        // blok K - kontrolujeme volnost
        if (blok->typ == Tblok::btK) {
            if (blok->r[TblokK::rel::J]) {
                stavOK = false;
                upoL.append(QString("obs %1").arg(blok->name));
            }
        }
        // blok KU - kolejové relé
        if (blok->typ == Tblok::btKU) {
            if (blok->r[TblokKU::rel::J]) {
                stavOK = false;
                upoL.append(QString("obs %1").arg(blok->name));
            }
        }
    };
    upoVolnosti.clear();
    this->upoVolnosti = upoL.join(',');
    return stavOK;
}

bool TdohledCesty::cestaPodDohledem::kontrolaPolohVymen()
{
    bool stavOK = true;
    QStringList upoL;
    upoL.clear();
    // kontrola pojížděných výhybek
    foreach (struct Tcesta::Tvyh vymena, pCesta->polohy) {
        // EMZ
        if (vymena.pBlok->typ == Tblok::btEMZ) {
            if (vymena.pBlok->r[TblokEMZ::UK]) {
                stavOK = false;
                upoL.append(QString("EMZ"));
            }
        }
        // Přestavník
        if (vymena.pBlok->typ == Tblok::btV) {
            TblokV *vyh = static_cast<TblokV*>(vymena.pBlok);
            TblokV *dvojiceBlok = vyh->dvojceBlok;
            // vždy kontroluje uvedenou výhybku
            if (!(vymena.minus) && (!(vyh->r[TblokV::rel::DP]))) {
                stavOK = false;
                upoL.append(QString("%1+").arg(vymena.pBlok->name));
            }
            if ( (vymena.minus) && (!(vyh->r[TblokV::rel::DM]))) {
                stavOK = false;
                upoL.append(QString("%1-").arg(vymena.pBlok->name));
            }
            if (dvojiceBlok != nullptr) {
                // pokud je dvojice, tak kontroluje i druhou půlku
                if (!(vymena.minus) && (!(dvojiceBlok->r[TblokV::rel::DP]))) {
                    stavOK = false;
                    upoL.append(QString("%1+").arg(dvojiceBlok->name));
                }
                if ( (vymena.minus) && (!(dvojiceBlok->r[TblokV::rel::DM]))) {
                    stavOK = false;
                    upoL.append(QString("%1-").arg(dvojiceBlok->name));
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
                upoL.append(QString("EMZ"));
            }
        }
        // Přestavník
        if (vymena->typ == Tblok::btV) {
            TblokV *vyh = static_cast<TblokV*>(vymena);
            TblokV *dvojiceBlok = static_cast<TblokV*>(vymena)->dvojceBlok;
            if (!(odv.minus) && (!(vyh->r[TblokV::rel::DP]))) {
                stavOK = false;
                upoL.append(QString("o%1+").arg(vyh->name));
            }
            if ((odv.minus) && (!(vyh->r[TblokV::rel::DM]))) {
                stavOK = false;
                upoL.append(QString("o%1-").arg(vyh->name));
            }
            if (dvojiceBlok != nullptr) {
                // pokud je dvojice, tak kontroluje i druhou půlku
                if (!(odv.minus) && (!(dvojiceBlok->r[TblokV::rel::DP]))) {
                    stavOK = false;
                    upoL.append(QString("o%1+").arg(dvojiceBlok->name));
                }
                if ( (odv.minus) && (!(dvojiceBlok->r[TblokV::rel::DM]))) {
                    stavOK = false;
                    upoL.append(QString("o%1-").arg(dvojiceBlok->name));
                }
            }
        }
    };
    // odstatění duplicit výhybka a odvrat
    QString name1;
    QString name2;
    for(int i = 0; i < upoL.count(); i++) {
        name1 = upoL.at(i);
        for(int j = 0; j < upoL.count(); j++) {
            name2 = upoL.at(j);
            if (name1 == name2.remove(name2.length()-1, 1)) {
                // stavíme výhybku, která je současně odvratem
                // necháme hlášení jen o odvratu
                upoL.removeAt(i);
                break;
            }
        }
    }
    // sestavý UPO
    upoPolohy.clear();
    this->upoPolohy = upoL.join(',');
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
// (kniha s.41-43)
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
                upoVOPVOM += QString("opak %1-,").arg(v->name);
            } else {
                if (v->r[TblokV::rel::VOM]) stavOK = false;
                upoVOPVOM += QString("opak %1+,").arg(v->name);
            }
        }
    }
    // to samé pro odvraty
    for(struct Tcesta::Tvyh_odv vc : pCesta->odvraty) {
        if (vc.pBlokVymena->typ == Tblok::btV) {
            TblokV *v = static_cast<TblokV *>(vc.pBlokVymena);
            if (vc.minus) {
                if (v->r[TblokV::rel::VOP]) stavOK = false;
                upoVOPVOM += QString("opak %1-,").arg(v->name);
            } else {
                if (v->r[TblokV::rel::VOM]) stavOK = false;
                upoVOPVOM += QString("opak %1+,").arg(v->name);
            }
        }
    }
    //if (upoVOPVOM.endsWith(",")) upoVOPVOM.removeLast();
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
    for (struct Tcesta::Tvyh_odv v : pCesta->odvraty) {
        log(QString("dohled: aktivace %1 na odvratné výměne %2")
                .arg((v.minus) ? "VOM" : "VOP", v.pBlokVymena->name),
            logging::LogLevel::Commands);
        if (v.pBlokVymena->typ == Tblok::btV) {
            // blok V se už nestaví
            v.pBlokVymena->r[TblokV::VOP] = !v.minus;
            v.pBlokVymena->r[TblokV::VOM] =  v.minus;
        }
    }
    // sepne výměnová automatická relé pro tlačítka
    for(int i = 0; i < pCesta->tlacitka.count(); i++) {
        TblokTC *t = pCesta->tlacitka[i];
        if (i == 0) t->r[TblokTC::PO] = true; // první tlačítko stále bliká
        t->r[TblokTC::VA] = true; // další tlačítka v cestě svítí trvale - výměnová automatika v činnosti
    }
    // zapne VA pro tlačítka mezilehlá
    for (TblokTC *t : pCesta->tlacitkaMezilehla) {
        t->r[TblokTC::rel::VA] = true;
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
    for (struct Tcesta::Tvyh_odv v : pCesta->odvraty) {
        log(QString("dohled: deaktivace VOP/VOM na odvratné výměne %2")
                .arg(v.pBlokVymena->name),
            logging::LogLevel::Commands);
        if (v.pBlokVymena->typ == Tblok::btV) {
            // blok V se už nestaví
            v.pBlokVymena->r[TblokV::VOP] = false;
            v.pBlokVymena->r[TblokV::VOM] = false;
        }
    }
    // vypne výměnová automatická relé
    for (TblokTC *t : pCesta->tlacitka) {
        t->r[TblokTC::rel::VA] =  false; // výměnová automatika vypnout
    }
    // vypne VA pro tlačítka mezilehlá
    for (TblokTC *t : pCesta->tlacitkaMezilehla) {
        t->r[TblokTC::rel::VA] = false;
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
    case scKontrolaDN: return QString("KontroDN"); break;
    case scDN: return QString("DN"); break;
    case scPrujezdVlaku: return QString("Prujezd"); break;
    case scPoruchaDN: return QString("Por.DN"); break;
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
    TblokKU *pBlokKU;
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

        // zrušení časovým souborem
        if (d->ruseni != rcNic) {
            bool casUplynul = false;
            // zjístí, zda spravný časovač odměřil čas
            if (d->ruseni == rc5) { casUplynul = rD5C; rZ5C = true; }
            if (d->ruseni == rc1) { casUplynul = rD1C; rZ1C = true; }
            if (d->ruseni == rc3) { casUplynul = rD3C; rZ3C = true; }
            if (casUplynul) {
                // vypne časovač - to se udělá samo
                /*
                if (d->ruseni == rc5) rZ5C = false;
                if (d->ruseni == rc1) rZ1C = false;
                if (d->ruseni == rc3) rZ3C = false;
                */
                log(QString("dohled: rušení uplynutím času cestu číslo %1").arg(d->num), logging::LogLevel::Commands);

                // zruší cestu
                for (Tblok *b : c->bloky) {
                    if (b->typ == Tblok::btS) {
                        b->r[TblokS::rel::Z] = false;
                    }
                }
                // cestu lze zrušit
                cestyNaSmazani.append(d);
            }
        }

        // konec rušícího povelu - puštění počátečního tlačítka
        if (d->povelRC && !c->tlacitka[0]->r[TblokTC::rel::RC]) {
            d->povelRC = false;
            c->Navestidlo->r[TblokQ::rel::Nv] = false;
            log(QString("dohle: konec rušení cesty číslo %1").arg(c->num), logging::LogLevel::Debug);
        }

        // vytáhnutí tlačítka ruší cestu, ale jen ve správných fázích
        bool ruseniOK = true;
        if (c->tlacitka[0]->r[TblokTC::rel::RC] && !d->ruseni) {
            // počáteční tlačítko je vytažené
            d->povelRC = true;
            if (d->stav == scDN) {
                // nejprve shodíme návestidlo
                if (c->Navestidlo->r[TblokQ::rel::N]) {
                    c->Navestidlo->r[TblokQ::rel::Nv] = true;
                    ruseniOK = false; // zatím nerušíme
                }
            }
            if (ruseniOK && ((d->stav == scZavery) || (d->stav == scKontrolaDN) || (d->stav == scDN))) {
                // spouští časovače pro rušení cest
                if (d->vlakEV) {
                    if (c->posun) {
                        if (!rZ1C) {
                            log(QString("dohled: rušení počátkem cesty číslo %1, zapnut časový soubor 1 min").arg(d->num), logging::LogLevel::Info);
                            rZ1C = true;
                            d->ruseni = rc1;
                            d->stav = scRC;
                        }
                    } else {
                        if (!rZ3C) {
                            log(QString("dohled: rušení počátkem cesty číslo %1, zapnut časový soubor 3 min").arg(d->num), logging::LogLevel::Info);
                            rZ3C = true;
                            d->ruseni = rc3;
                            d->stav = scRC;
                        }
                    }
                } else {
                    if (!rZ5C) {
                        log(QString("dohled: rušení počátkem cesty číslo %1, zapnut časový soubor 5 sek").arg(d->num), logging::LogLevel::Info);
                        rZ5C = true;
                        d->ruseni = rc5;
                        d->stav = scRC;
                    }
                }
            }
        }

        // stisknutí tlačítka může obnovit DN, když je správná situace
        if (c->tlacitka[0]->mtbIns[TblokTC::mtbeIns::mtbInVolba].value()) {
            // počáteční tlačítko je stisknuté
            if ((d->stav == scPrujezdVlaku) || (d->stav == scDN) || (d->stav == scRC)) {
                // pokud jsou splněny podmínky, přejdeme na znovuzapnutí DN
                if (d->kontrolaCelistvostiCesty()) {
                    // obnovíme cestu
                    if (d->stav == scRC) {
                        if (d->ruseni == rc5) rZ5C = false;
                        if (d->ruseni == rc1) rZ1C = false;
                        if (d->ruseni == rc3) rZ3C = false;
                        d->ruseni = rcNic;
                    }
                    d->stav = scKontrolaDN;
                    log(QString("dohled: obnova DN cesty číslo %1").arg(d->num), logging::LogLevel::Info);
                }
            }
        }

        // rušení stavění tlačítkem RVN
        if (voliciskupina.mtbInRuseniVolby.value() || (rDCCVypadek) || (rDCCZkrat)) {
            switch (d->stav) {
            case scStavime:
            case scZavery:
                d->povelVAvypnout(); // nezapomenou vypnout VA !
                [[fallthrough]];
            case scZvoleno:
                cestyNaSmazani.append(d);
                break;
            default:
                ;
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
            if (d->upoZavery.length() > 0) d->upo.append(d->upoZavery);
            if (d->upoVOPVOM.length() > 0) d->upo.append(d->upoVOPVOM);
            // povel pro zrušení od volící skupiny
            // povel pro stavění
            if (stavOK) {
                // sepne VOP/VOM a výměnová automatická relé
                d->povelVAzapnout();
                // posun do dalšího stavu
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scStavime)), logging::LogLevel::Commands);
                voliciskupina.probihaVolba = false; // volící skupina je připravena na další volbu
                d->stav = scStavime;
            }

            break;
        case scStavime: // kontrola výměn
            // kontrola všech výhybek v cestě i odvratů, zda už mají polohu
            stavOK = d->kontrolaPolohVymen();
            stavOK &= d->kontrolaZavery(false);
            if (d->upoZavery.length() > 0) d->upo.append(d->upoZavery);
            if (d->upoPolohy.length() > 0) d->upo.append(d->upoPolohy);

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
            if (d->upoZavery.length() > 0) d->upo.append(d->upoZavery);
            if (d->upoPolohy.length() > 0) d->upo.append(d->upoPolohy);
            // povel pro zrušení od volící skupiny - stále můžeme
            if (stavOK) {
                // úseky jsou volné a nemají závěr z jiné cesty
                log(QString("dohled: provedeme závěr celé cesty číslo %1").arg(d->num), logging::LogLevel::Commands);
                // vše v pořádku, provedeme závěr cesty
                for (int i = 1; i < c->bloky.length(); i++) {
                    Tblok *blok = c->bloky[i];
                    if (blok->typ == Tblok::btS) {
                            blok->r[TblokS::Z] = true; // aktivujeme závěrná relé
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
                    // kolejové relé - pokud je v ceste, tak jde o odjezdovou cestu
                    // zablokujeme zvonek předhlášky
                    if (blok->typ == Tblok::btKU) {
                        pBlokKU = static_cast<TblokKU *>(blok);
                        pBlokKU->r[TblokKU::rel::OC] = true; // odjezdová cesta (je nebo bude)
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

                log(QString("dohled: konec výměnové automatiky"), logging::LogLevel::Commands);
                d->povelVAvypnout();

                //log(QString("dohled: zhasne tlačítka cesty"), logging::LogLevel::Commands);
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
                    c->Navestidlo->r[TblokQ::rel::N] = true;
                    d->vlakCelo = 0;
                    d->vlakKonec = 0;
                    d->vlakEvidenceCelo = false;
                    d->vlakEvidenceKonec = false;
                    d->vlakEV = false;
                    d->povelPOvypnout();
                    log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scDN)), logging::LogLevel::Commands);
                    d->stav = scDN;
                }
            }
            break;
        case scDN:
            // vyhodnocení EV
            if ((!(d->vlakEV) && c->zjistiObsazeni(0))) {
                d->vlakEV = true;
            }

            stavOK = d->kontrolaCelistvostiCesty();
            if (stavOK) {
                if (c->Navestidlo) {
                    if (c->Navestidlo->r[TblokQ::N]) {
                        // DN - urči návěst
                        c->Navestidlo->navestniZnak = urciNavest(c->navZnak, c->nasledneNavestidlo);
                    } else {
                        // není DN - nevíme proč, ale necháme to tak
                        // může být zrušeno vytažením počátku
                    }
                }
            } else {
                if (!(c->posun)) {
                    if (c->Navestidlo) {
                        c->Navestidlo->r[TblokQ::rel::Nv] = true;
                        log(QString("dohled: cesta č. %1 zhoď návěstidlo %2").arg(d->num).arg(c->Navestidlo->name), logging::LogLevel::Info);
                    }
                }
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scPrujezdVlaku)), logging::LogLevel::Commands);
                d->stav = scPrujezdVlaku;
            }
            break;
        case scPoruchaDN:
        case scPrujezdVlaku:
            // shození návestidla pro posunovou cestu
            // ToDo: jen při obs. úseku před návěstidle, jinak při uvolnění před náv.
            if (c->posun) {
                if (c->Navestidlo && (c->Navestidlo->r[TblokQ::N] == true)) {
                    if (c->zjistiObsazeni(0)) {
                        // obsazený úsek před návěstidlem
                        if (d->vlakKonec != 1) {
                            // závěr v 1. úseku vypbaven
                            if (c->Navestidlo->r[TblokQ::Nv] == false) {
                                log(QString("dohled: cesta č. %1 zhoď seř. náv. %2 - uvolnění 1. úseku").arg(d->num).arg(c->Navestidlo->name), logging::LogLevel::Info);
                            }
                            if (c->Navestidlo->r[TblokQ::N]) {
                                c->Navestidlo->r[TblokQ::Nv] = true;
                            }
                        } else {
                            c->Navestidlo->r[TblokQ::Nv] = false;
                        }
                    } else {
                        // volný úsek před návěstidlem
                        // zhodit návěst
                        if (!c->Navestidlo->r[TblokQ::Nv]) {
                            log(QString("dohled: cesta č. %1 zhoď seř. náv. %2 - volný úsek před náv.").arg(d->num).arg(c->Navestidlo->name), logging::LogLevel::Info);
                        }
                        if (c->Navestidlo->r[TblokQ::N]) {
                            c->Navestidlo->r[TblokQ::Nv] = true;
                        }

                    }
                }
            }
            // vyhodnocení EV
            if ((!(d->vlakEV) && c->zjistiObsazeni(0))) {
                d->vlakEV = true;
                log(QString("dohled: cesta č. %1 evidence vlaku před návěstidlem").arg(d->num), logging::LogLevel::Info);
            }

            // pokud nejsme na konci, hýbene s čelem vlaku
            if (d->vlakCelo < c->bloky.count()) {
                obs1 = c->zjistiObsazeni(d->vlakCelo);
                obs2 = c->zjistiObsazeni(d->vlakCelo+1);
                if (obs1 && obs2) {
                    if (d->vlakCelo == 0) {
                        d->vlakEvidenceCelo = true;
                        d->vlakCelo = 0; // vlak za návestidlem, ne před
                    }
                    d->vlakCelo++;
                    log(QString("dohled: cesta č. %1 čelo vlaku +1 = %2").arg(d->num).arg(d->vlakCelo), logging::LogLevel::Info);
                }
            }

            // inicializace konce při průjezdu
            if ((d->vlakCelo == 1) && (d->vlakKonec == 0)) {
                d->vlakKonec = 1;
                log(QString("dohled: cesta č. %1 konec vlaku inicializace").arg(d->num), logging::LogLevel::Info);
            }

            // pokud nejsme na konci, hýbene s koncem vlaku
            if ((d->vlakKonec < c->bloky.count()) && (d->vlakKonec < d->vlakCelo)) {
                obs1 = c->zjistiObsazeni(d->vlakKonec);
                obs2 = c->zjistiObsazeni(d->vlakKonec+1);
                if (!obs1 && obs2) {
                    if (config.cfgVybav) {
                        // postupné rušení závěrů
                        c->uvolniZaver(d->vlakKonec);
                    }
                    d->vlakKonec++;
                    log(QString("dohled: cesta č. %1 konec vlaku +1 = %2").arg(d->num).arg(d->vlakKonec), logging::LogLevel::Info);
                }
            }

            if ((d->vlakCelo == c->bloky.count()) && ((d->vlakKonec+1) == c->bloky.count())) {
                log(QString("dohled: cesta č. %1 změna stavu %2-%3").arg(d->num).arg(stavCesty2QString(d->stav)).arg(stavCesty2QString(scProjeto)), logging::LogLevel::Commands);
                // pokud jsem zkončili v blok M, musíme v něm zrušit závěr
                if (c->bloky.last()->typ == Tblok::btS) {
                    c->bloky.last()->r[TblokS::rel::Z] = false;
                }
                d->stav = scProjeto;
            }
            break;
        case scProjeto:
            if (config.cfgVybav) {
                log(QString("dohled: cesta č. %1 kompletně vybavena").arg(d->num), logging::LogLevel::Commands);
                cestyNaSmazani.append(d);
            } else {
                // zrušit cesty, při stisku tlačítka RC
                // blok RC vyřeší zbytek
                // indikace cesty projeté
                for(int i = 1; i < c->bloky.count(); i++) {
                    if (c->bloky[i]->typ == Tblok::btS) {
                        c->bloky[i]->r[TblokS::rel::P] = true;
                    }
                }
            }
            break;
        case scRC: // RC
            c->Navestidlo->r[TblokQ::Nv] = true;
            // pokud se poruší podmínky cesty, zruší se rušení časovým souborem
            if (!d->kontrolaCelistvostiCesty()) {
                // zrušíme rušení časovým soborem
                if (d->stav == scRC) {
                    if (d->ruseni == rc5) rZ5C = false;
                    if (d->ruseni == rc1) rZ1C = false;
                    if (d->ruseni == rc3) rZ3C = false;
                    d->ruseni = rcNic;
                }
            }
            d->stav = scRC;
            break;
        case scZbytek: // bad state
            d->upo.append("nekompletni");
            if (c->Navestidlo->r[TblokQ::N]) {
                c->Navestidlo->r[TblokQ::Nv] = true;
            }
            if (d->kontrolaZavery(false)) {
                log(QString("dohled: cesta č. %1 se rozpadla, nutné NUZ").arg(d->num), logging::LogLevel::Commands);
                cestyNaSmazani.append(d);
            }
            break;
        }
    }
    // vybaví určené cesty
    for(cestaPodDohledem *d : cestyNaVybaveni) {
        if (d->stav >= scProjeto) {
            log(QString("dohled: cesta č. %1 rušení závěrů po projetí").arg(d->num), logging::LogLevel::Commands);
            for(Tblok *bl : c->bloky) {
                if (bl->typ == Tblok::btS) {
                    static_cast<TblokS *>(bl)->r[TblokS::rel::Z] = false;
                }
            }
            // ukoncí indikaci cesty projeté
            for(int i = 1; i < c->bloky.count(); i++) {
                if (c->bloky[i]->typ == Tblok::btS) {
                    c->bloky[i]->r[TblokS::rel::P] = true;
                }
            }
            log(QString("dohled: cesta č. %1 vybaveno").arg(d->num), logging::LogLevel::Commands);
            cestyNaSmazani.append(d);
            cestyNaVybaveni.removeOne(d);
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
                d->povelPOvypnout();
            }
        }
        if (d->stav == scStavime) {
            d->povelVAvypnout();
            d->povelPOvypnout();
        }
        if (d->stav >= scZavery) {
            c->Navestidlo->r[TblokQ::Nv] = true;
        }
        // a smazat cestu
        log(QString("dohled: cesta č. %1 zrušit").arg(d->num), logging::LogLevel::Commands);
        cestyPostavene.removeOne(d);
        delete d;
    }
}

TdohledCesty dohledCesty;
