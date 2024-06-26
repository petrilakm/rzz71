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

    enum bloktyp {btNULL, btTC, btPr, btV, btS, btK, btQ, btSimV};

    virtual bool evaluate(); // if change return true

    QList<bool> r; // relatka
    mtbpin mtbIns[BLOK_MTB_MAX];
    mtbpin mtbOut[BLOK_MTB_MAX];
    QString name;
    enum bloktyp typ = btNULL;

    static Tblok* findBlokByName(QString name);
};

//extern Tvoliciskupina *pvoliciskupina;

/* Seznam bloků:
 *
 * blokV  - výhybka
 * blokK  - staniční kolej
 * blokM  - vyhýbkový úsek
 * blokTC - tlačítko cestové
 * blokUV - úvazka (traťové souhlasy, místní i vzdálená)
 *
 */

extern QList<Tblok *> bl;

#endif // BLOK_H
