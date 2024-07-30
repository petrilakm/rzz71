#include "blokEMZ.h"

TblokEMZ::TblokEMZ() {
    typ = btEMZ;
    for (int i = 0; i < RELAY_COUNT_EMZ; ++i) {
        r.append(false);
    }
}

bool TblokEMZ::evaluate()
{
    QList<bool> rLast = r;
    // logika



    if (r != rLast) return true; else return false;
}
