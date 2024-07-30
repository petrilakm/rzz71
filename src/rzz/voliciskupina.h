#ifndef VOLICISKUPINA_H
#define VOLICISKUPINA_H

#include <QList>
#include "rzz/obecne.h"
//#include "rzz/blok.h"
#include "rzz/blokTC.h"
#include "rzz/cesty.h"



class Tvoliciskupina
{
public:
    Tvoliciskupina();
    //QList<Tcesta*> cesty;
    QList<int> cestyPostavene;
    QList<int> cestyMozne;
    QList<TblokTC *> tlacitkaAktivni;
    //bool bProbihaVolba;
    //TblokTC *pocatek; // ukazatel na první počátek cesty

    mtbpin mtbInRuseniVolby;
    mtbpin mtbOutProbihaVolba;

    void ruseniVolbyCesty();
    //bool kontrolaTZ(TblokTC *pTC);
    //bool pocatekJeVCeste(Tblok *p);
    bool vstupZmena(TblokTC *p, bool state);
    void postavCestu(int i);
    bool evaluate();
};

extern Tvoliciskupina voliciskupina;

#endif // VOLICISKUPINA_H
