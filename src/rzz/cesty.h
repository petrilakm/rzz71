#ifndef CESTY_H
#define CESTY_H

#include "rzz/blokTC.h"
#include "rzz/blokV.h"
#include "rzz/blokK.h"
#include "rzz/blokS.h"
#include "rzz/blokQ.h"
#include "rzz/blokKU.h"

class Tcesta
{
public:
    struct Tvyh {
        bool minus;
        Tblok *pBlok;
    };

    // odvrat, co můžeme kontrolovat
    struct Tvyh_odv {
        bool minus;
        Tblok *pBlokVymena;
        Tblok *pBlokUsek;
    };

    //Tblok *pocatek;
    QList<TblokTC *> tlacitka;
    QList<struct Tvyh> polohy;
    QList<struct Tvyh_odv> odvraty;
    QList<TblokTC *>tlacitkaMezilehla;
    QList<Tblok *> bloky;
    TblokQ *Navestidlo;
    TblokQ *nasledneNavestidlo;
    int navZnak; // vávěstní znak, ideální
    int num; // číslo cesty v zav. tabulce
    bool posun; // jde o posun? jinak vlaková

    bool zjistiObsazeni(int usek);
    bool zjistiZaver(int usek);
    void uvolniZaver(int usek);
};

class Tcesty
{
public:
    Tcesty();
    QList<Tcesta*> cesty;

    void load();
};

extern Tcesty *cesty;

#endif // CESTY_H
