#ifndef BLOK_H
#define BLOK_H

#include "obecne.h"

class mtbpin;

#define BLOK_MTB_MAX (16)

class Tblok : public QObject
{
    Q_OBJECT
public:
    Tblok();

    enum bloktyp {btNULL, btTC, btPr, btEMZ, btV, btS, btK, btQ, btSimV, btPN, btTS, btRC, btOs};

    virtual bool evaluate(); // if change return true

    QList<bool> r; // relatka
    mtbpin mtbIns[BLOK_MTB_MAX];
    mtbpin mtbOut[BLOK_MTB_MAX];
    QString name;
    enum bloktyp typ = btNULL;
    bool bBlikUsed; // zda používáme kmitač

    static Tblok* findBlokByName(QString name);
};

//extern Tvoliciskupina *pvoliciskupina;

/* Seznam bloků:
 *
 * blokV  - výhybka
 * blokK  - staniční kolej
 * blokS  - vyhýbkový úsek (S nebo M)
 * blokPr - průsvitka
 * blokQ  - návěstidlo
 * blokPN - přivolávací návest - obslužné tlačítko
 * blokEMZ- elektromagnetický zámek - simulace
 * blokTC - tlačítko cestové (počátecní i koncové)
 * blokTS - úvazka (traťové souhlasy, místní i vzdálená)
 * blokPN - přivolávací návest na odjezdová nívěstidla
 * blokRC - rušení cesty po projetí - místo automatického rozpadu cesty
 *
 */

extern QList<Tblok *> bl;

#endif // BLOK_H
