#include "blokOs.h"

TblokOs::TblokOs(QObject *parent) : Tblok{parent}
{
    typ = btOs;
    for (int i = 0; i < RELAY_COUNT_OS; ++i) {
        r.append(false);
    }
}

bool TblokOs::evaluate()
{
    QList<bool> rLast = r;

    r[rel::OSV] = mtbIns[mtbInOsv].value();
    mtbOut[mtbOutInd].setValueBool(r[rel::OSV]);
    mtbOut[mtbOutOsv].setValueBool(r[rel::OSV]);
    // speciální případ
    if (this->name == "NavN") {
        rNavNoc = r[rel::OSV];
    }

    if (r != rLast) return true; else return false;
}
