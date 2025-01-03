#include "blokTC.h"
#include "voliciskupina.h"

TblokTC::TblokTC(QObject *parent) : Tblok{parent} {
    typ = btTC;
    for (int i = 0; i < RELAY_COUNT_TC; ++i) {
        r.append(false);
    }
}

bool TblokTC::evaluate()
{
    QList<bool> rLast = r;
    
    bool mtbVolba = mtbIns[mtbInVolba].value();
    bool mtbZrus  = mtbIns[mtbInRuseni].value();
    bool platnaVolba = false;
    if (mtbVolba && !r[TZ] && !r[VA] && !mtbVolbaOpak && !r[PO] && !r[NM]) {
        mtbVolbaOpak = true;
        log("blokTC: stisk tlačítka", logging::LogLevel::Debug);
        platnaVolba = voliciskupina.vstupZmena(this, true);
        r[TZ] = platnaVolba;
    }
    if (!mtbVolba) {
        mtbVolbaOpak = false;
    }
    r[RC] = (mtbZrus && !r[ZFo]);
    if (mtbZrus && !r[NM]) {
        // tlačítko vytačeno, ale není od něj zádná cesta postavená
        log("blokTC: vytažení tlačítka", logging::LogLevel::Debug);
        platnaVolba = voliciskupina.vstupZmena(this, false);
        r[TZ] = false;
        r[PO] = false;
    }

    if (r[VA]) {
        r[TZ] = false;
    }

    // návěstní relé zruší TZ i PO
    if (r[NM]) {
        r[TZ] = false;
        r[PO] = false;
    }

    // indikace na pultu (kniha s. 33)
    bool out = false;
    if (r[TZ] ^ r[PO]) {
        out = rBlik50;
    } else {
        if (!(r[TZ] || r[PO])) {
            out = r[VA];
        }
    }

    if (out != mtbOut[mtbOutIndikace].valueOutBool()) {
        mtbOut[mtbOutIndikace].setValue(out);
    }
    if (r != rLast) return true; else return false;
}
