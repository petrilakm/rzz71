#include "blokS.h"

TblokS::TblokS(QObject *parent) : Tblok{parent} {
    typ = btS;
    typM = false;
    for (int i = 0; i < RELAY_COUNT_S; ++i) {
        r.append(false);
    }
    // časovač pro zpoždění uvolnění závěru
    zpozdeniReleZ = new QTimer(this);
    zpozdeniReleZ->setSingleShot(true);
    zpozdeniReleZ->setInterval(config.tUvolneniZ);
    connect(zpozdeniReleZ, SIGNAL(timeout()), this, SLOT(slotReleZpritah()));
}

bool TblokS::evaluate()
{
    QList<bool> rLast = r;
    // logika

    // obsazení úseku - ošetřen výpadek DCC
    if (mtbIns[mtbInObsaz].valid) {
        if (!(rDCCVypadek || rDCCZkrat)) {
            r[J] = mtbIns[mtbInObsaz].value();
        }
    } else {
        r[J] = true;
    }

    // logika nouzového rušení závěru
    r[V] |= (mtbIns[mtbInNuz].value() && (r[Z]) && (!rQTV));
    r[V] &= (r[Z]);
    //r[Z] = r[A] || r[B];
    if (r[V]) {
        r[R] = (rD3V); // R je sepnuto časovým souborem
    } else {
        r[R] = false;
    }
    r[Z] &= !r[R]; // R ruší Z
    r[P] &= !r[V] && r[Z]; // V ruší P a shozené Z

    // logika průsvitek bílých
    if ((r[Z])) {
        r[PrB] = !r[J] && ((r[V]) ? rBlik50 : 1) && ((!r[P] || rBlik100));
    } else {
        r[PrB] = !typM && rKPV;
    }
    // logika průsvitek červených
    r[PrC] = r[J];

    if (r != rLast) return true; else return false;
}

void TblokS::zrusZaver()
{
    if (!(zpozdeniReleZ->isActive())) {
        zpozdeniReleZ->start();
    }
}

void TblokS::slotReleZpritah()
{
    r[Z] = false;
}
