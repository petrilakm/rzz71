#include "blokTS.h"

TblokTS::TblokTS()
{
    typ = btTS;
    casovacObsluhy = new QTimer(this);
    casovacObsluhy->setSingleShot(true);
    casovacObsluhy->setInterval(2000);
    for (int i = 0; i < RELAY_COUNT_TS; ++i) {
        r.append(false);
    }
}

bool TblokTS::evaluate()
{
    bool vyzadovanaObsluha = false;

    // logika

    if (vyzadovanaObsluha) {
        casovacObsluhy->start();
        vyzadovanaObsluha = false;
    }
    return false;
}
