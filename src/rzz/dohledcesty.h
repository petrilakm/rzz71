#ifndef DOHLEDCESTY_H
#define DOHLEDCESTY_H

/*
 Dohled nad cestami, co se staví a co jsou již postaveny
 nahrazuje výměnová automatická rele, souhlasy,
 kontrolu celistvosti cesty a postupný průjezd vlakem
*/

#include <QList.h>
#include "rzz/obecne.h"
#include "rzz/blokQ.h"
#include "rzz/cesty.h"

class TdohledCesty
{
public:
    TdohledCesty();
    enum stavCesty {
        scStavime = 0,
        scZavery = 1,
        //scKontrolaDN = 2,
        scDN = 3,
        scPrujezdVlaku = 4,
        scRC = 5
    };
    // scStavime - výměnová autoamatická relé - aktivní VOP a VOM
    // scZavery - výměny přestaveny, kontrolujeme podmínky pro závěr
    // scKontrolaDN - čekáme na splnění podmínek pro DN
    // scDN - vše v pořídku, DN na návěstidle, čekáme na vlak
    // scPrujezdVlaku - vlak již jede, cesta se postupně rozpadá
    // scRC - nouzové rušení celé cesty, čekáme na časový soubor
    struct cestaPodDohledem {
        enum stavCesty stav; // akktuální stav cesty
        int num; // číslo cesty (id)
        int vlakCelo; // pořadové číslo bloku, kde je čelo vlaku
        int vlakKonec; // pořadové číslo bloku, kde je konec vlaku
        bool vlakEvidenceCelo;
        bool vlakEvidenceKonec;
    };

    QList<struct cestaPodDohledem *> cestyPostavene;

    void postavCestu(int i); // předání cesty z volící skupiny
    void evaluate(); // pravidelná kontrola podmínek
    void t3C(); // uplynul časový soubor 3 min
    void t1C(); // uplynul časový soubor 1 min
    void t5C(); // uplynul časový soubor 5 sec

    void zhasniTlacitka(int i); // číslo cesty, kde zhasínáme

    int urciNavest(int navZnak, TblokQ *nasledneNavestidlo = nullptr);
    bool kontrolaCelistvostiCesty(Tcesta *c);
};

extern TdohledCesty dohledCesty;

#endif // DOHLEDCESTY_H
