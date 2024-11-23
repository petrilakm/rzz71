#ifndef DOHLEDCESTY_H
#define DOHLEDCESTY_H

/*
 Dohled nad cestami, co se staví a co jsou již postaveny
 nahrazuje výměnová automatická rele, souhlasy,
 kontrolu celistvosti cesty a postupný průjezd vlakem
*/

#include <qlist.h>
#include "rzz/obecne.h"
#include "rzz/blokQ.h"
#include "rzz/cesty.h"

class TdohledCesty
{
public:
    TdohledCesty();
    enum stavCesty {
        scZvoleno = 0, // víme jakou cestu stavíme
        scStavime = 1, // cesta stále ve volící skupině, výměny
        scZavery = 2, // padání závěrů
        scKontrolaDN = 3, // cesta je již postavená
        scDN = 4,
        scPrujezdVlaku = 5, // už se nedá zrušit vytažením počátku
        scProjeto = 6, // již projetá cesta
        scRC = 7,
        scZbytek = 8 // zbytek po cestě, lze pouze zrušit NUZem
    };
    // scStavime - výměnová automatická relé - aktivní VOP a VOM
    // scZavery - výměny přestaveny, kontrolujeme podmínky pro závěr
    // scKontrolaDN - čekáme na splnění podmínek pro DN
    // scDN - vše v pořídku, DN na návěstidle, čekáme na vlak
    // scPrujezdVlaku - vlak již jede, cesta se postupně rozpadá
    // scRC - nouzové rušení celé cesty, čekáme na časový soubor

    QString stavCesty2QString(stavCesty sc);

    class cestaPodDohledem {
    public:
        enum stavCesty stav; // akktuální stav cesty
        int num; // číslo cesty (id)
        Tcesta *pCesta;
        int vlakCelo; // pořadové číslo bloku, kde je čelo vlaku
        int vlakKonec; // pořadové číslo bloku, kde je konec vlaku
        bool vlakEvidenceCelo;
        bool vlakEvidenceKonec;
        QStringList upo;
        QString upoZavery;
        QString upoVolnosti;
        QString upoPolohy;
        QStringList upoVOPVOM;
        bool kontrolaCelistvostiCesty();
        bool kontrolaZavery(bool cestaKompletni);
        bool kontrolaVolnosti();
        bool kontrolaPolohVymen();
        bool kontrolaNUZ();
        bool kontrolaVOPVOM();
        void povelVAzapnout();
        void povelVAvypnout();
        void povelPOvypnout();
    };

    QList<cestaPodDohledem *> cestyPostavene;

    void postavCestu(int i); // předání cesty z volící skupiny
    void evaluate(); // pravidelná kontrola podmínek
    void t3C(); // uplynul časový soubor 3 min
    void t1C(); // uplynul časový soubor 1 min
    void t5C(); // uplynul časový soubor 5 sec

    void zhasniTlacitka(int i); // číslo cesty, kde zhasínáme

    int urciNavest(int navZnak, TblokQ *nasledneNavestidlo = nullptr);
};

extern TdohledCesty dohledCesty;

#endif // DOHLEDCESTY_H
