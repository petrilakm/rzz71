#ifndef CESTY_H
#define CESTY_H

#include "rzz/blokTC.h"
#include "rzz/blokV.h"

class Tcesta
{
public:
    struct Tvyh {
        bool minus;
        TblokV *pBlokV;
    };

    //Tblok *pocatek;
    QList<TblokTC *> tlacitka;
    QList<struct Tvyh> polohy;
    int num;
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
