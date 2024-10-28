#include "blokTC.h"
#include "voliciskupina.h"

TblokTC::TblokTC() {
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
    if (mtbVolba && !r[TZ] && !r[PO] && !r[TK] && !mtbVolbaOpak) {
        mtbVolbaOpak = true;
        log("blokTC: stisk tlačítka", logging::LogLevel::Debug);
        bool platnaVolba = voliciskupina.vstupZmena(this, true);
        r[TZ] = platnaVolba;
    }
    if (!mtbVolba) {
        mtbVolbaOpak = false;
    }
    if (mtbZrus && r[TZ] && !r[PO] && !r[TK]) {
        //r[TZ] = false;
        log("blokTC: vytažení tlačítka", logging::LogLevel::Debug);
        voliciskupina.vstupZmena(this, false);
    }
    bool out = (r[TZ] || r[PO]) ? rBlik50 : false;
    out |= r[TK];
    if (out != mtbOut[mtbOutIndikace].valueOutBool()) {
        mtbOut[mtbOutIndikace].setValue(out);
    }
    if (r != rLast) return true; else return false;
}
