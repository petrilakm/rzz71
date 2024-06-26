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
    if (mtbVolba && !r[TZ] && !r[TK]) {
        //r[TK] = true;
        log("blokTC: probíhá volba", logging::LogLevel::Debug);
        bool platnaVolba = voliciskupina.vstupZmena(this, false);
        r[TZ] = platnaVolba;
    }
    if (mtbZrus && r[TZ] && !r[TK]) {
        r[TZ] = false;
        log("blokTC: volba zrušena", logging::LogLevel::Debug);
    }
    bool out = (r[TZ]) ? rBlik50 : false;
    out |= r[TK];
    if (out != mtbOut[mtbOutIndikace].valueOutBool()) {
        mtbOut[mtbOutIndikace].setValue(out);
    }
    if (r != rLast) return true; else return false;
}
