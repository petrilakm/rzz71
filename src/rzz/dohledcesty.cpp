#include "dohledcesty.h"
#include "cesty.h"
#include "rzz/blokK.h"

TdohledCesty::TdohledCesty(){
    cestyPostavene.clear();
}

void TdohledCesty::postavCestu(int i) {
    struct cestaPodDohledem *nc = new cestaPodDohledem(); // nová cesta
    nc->num = i;
    nc->stav = scStavime;
    nc->vlakCelo = -1;
    nc->vlakKonec = -1;
    cestyPostavene.append(nc);
}

void TdohledCesty::evaluate()
{
    Tcesta *c;
    TblokS *pBlokS;
    TblokK *pBlokK;
    bool stavOK;
    foreach (struct cestaPodDohledem *d, cestyPostavene) {
        c = cesty->cesty.at(d->num);
        switch (d->stav) {
        case scStavime: // kontrola výměn
            stavOK = true;
            foreach (struct Tcesta::Tvyh vymena, c->polohy) {
                if ((!vymena.minus && !vymena.pBlokV->r[TblokV::DP]) ||
                    ( vymena.minus && !vymena.pBlokV->r[TblokV::DM])) {
                    stavOK = false;
                }
            };
            if (stavOK) {
                log(QString("dohled: konec stavění výměn"), logging::LogLevel::Info);
                // máme postaveno, můžeme zrušit výměnová automatická relé a VOP, VOM
                foreach (struct Tcesta::Tvyh vymena, c->polohy) {
                    log(QString("dohled: výměnu %1 už nestavíme").arg(vymena.pBlokV->name), logging::LogLevel::Info);
                    vymena.pBlokV->r[TblokV::VOP] = false;
                    vymena.pBlokV->r[TblokV::VOM] = false;
                };
                // cesta se posune do dalšího stavu
                d->stav = scZavery;
            }
            break;
        case scZavery:
            stavOK = true;
            foreach (Tblok *blok, c->bloky) {
                if (blok->typ == Tblok::btS) {
                    pBlokS = static_cast<TblokS *>(blok);
                    if (pBlokS->r[TblokS::J] || pBlokS->r[TblokS::A] || pBlokS->r[TblokS::B]) stavOK = false;
                }
            };
            if (stavOK) {
                log(QString("dohled: provedeme závěr celé cesty číslo%1").arg(d->num), logging::LogLevel::Info);
                // vše v pořádku, provedeme závěr cesty
                foreach (Tblok *blok, c->bloky) {
                    if (blok->typ == Tblok::btS) {
                        pBlokS = static_cast<TblokS *>(blok);
                        pBlokS->r[TblokS::A] = true; // aktivujeme závěrná relé
                        pBlokS->r[TblokS::B] = true;
                    }
                    if (blok->typ == Tblok::btK) {
                        pBlokK = static_cast<TblokK *>(blok);
                        pBlokK->r[TblokK::X1] = true; // aktivujeme výluková relé
                        pBlokK->r[TblokK::X2] = true;
                    }
                };
                d->stav = scKontrolaDN;
            }
            break;
        default: // bad states
            d->stav = scRC;
            break;
        }
    }
}

TdohledCesty dohledCesty;
