#ifndef DOHLEDCESTY_H
#define DOHLEDCESTY_H

/*
 Dohled nad cestami, co se staví a co jsou již postaveny
 nahrazuje výměnová automatická rele, souhlasy,
 kontrolu celistvosti cesty a postupný průjezd vlakem
*/

#include <QList.h>
#include "rzz/obecne.h"

class TdohledCesty
{
public:
    TdohledCesty();
    enum stavCesty { scStavime, scZavery, scKontrolaDN, scDN, scPrujezdVlaku, scRC};
    // scStavime - výměnová autoamatická relé - aktivní VOP a VOM
    // scZavery - výměny přestaveny, kontrolujeme podmínky pro závěr
    // scKontrolaDN - čekáme na splnění podmínek pro DN
    // scDN - vše v pořídku, DN na návěstidle, čekáme na vlak
    // scPrujezdVlaku - vlak již jede, cesta se postupně rozpadá
    // scRC - nouzové rušení celé cesty, čekáme na časový soubor
    struct cestaPodDohledem {
        enum stavCesty stav; // akktuální stav cesty
        int num; // číslo cesty
        int vlakCelo; // číslo bloku, kde je čelo vlaku
        int vlakKonec; // číslo bloku, kde je konec vlaku
    };

    QList<struct cestaPodDohledem *> cestyPostavene;

    void postavCestu(int i); // předání cesty z volící skupiny
    void evaluate(); // pravidelná kontrola podmínek
};

extern TdohledCesty dohledCesty;

#endif // DOHLEDCESTY_H
