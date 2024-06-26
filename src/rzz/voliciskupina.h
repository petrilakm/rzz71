#ifndef VOLICISKUPINA_H
#define VOLICISKUPINA_H

#include <QList>
//#include "rzz/obecne.h"
//#include "rzz/blok.h"
#include "rzz/blokTC.h"
#include "rzz/cesty.h"



class Tvoliciskupina
{
public:
    Tvoliciskupina();
    //QList<Tcesta*> cesty;
    QList<int> cestyPostavene;
    bool bProbihaVolba;
    QString sPocatek;
    TblokTC *pocatek; // ukazatel na první počátek cesty

    void ruseniVolbyCesty();
    //bool kontrolaTZ(TblokTC *pTC);
    //bool pocatekJeVCeste(Tblok *p);
    bool vstupZmena(TblokTC *p, bool state);
};



#endif // VOLICISKUPINA_H
