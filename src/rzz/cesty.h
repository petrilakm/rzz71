#ifndef CESTY_H
#define CESTY_H

#include "rzz/blokTC.h"
#include "rzz/blokV.h"
#include "rzz/blokQ.h"

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
    QList<Tblok *> bloky;
    TblokQ *Navestidlo;
    TblokQ *nasledneNavestidlo;
    int navZnak;
    int num;
    bool posun;
    //Tblok *navestidlo;
    //QList<TblokU> useky;
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
