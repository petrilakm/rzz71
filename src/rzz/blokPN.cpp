#include "blokPN.h"

TblokPN::TblokPN() {
    typ = btPN;
    for (int i = 0; i < RELAY_COUNT_PN; ++i) {
        r.append(false);
    }
    tim = new QTimer(this);
    tim->setSingleShot(true);
    connect(tim, &QTimer::timeout, this,  &TblokPN::on_tim);
}

bool TblokPN::evaluate()
{
    QList<bool> rLast = r;
    // logika

    if (mtbIns[mtbInPNEnable].valid) {
        // odjezd
        r[PN] = mtbIns[mtbInPN].value() && mtbIns[mtbInPNEnable].value();
    } else {
        //vjezd
        r[PN] = mtbIns[mtbInPN].value();
    }

    if (r[PN] && !rLast[PN]) {
        // došlo k aktivaci přiloválací návesti
        mtbOut[mtbOutPocitadlo].setValueBool(true);

        tim->start(800);
    }

    if (!r[PN] && rLast[PN]) {
        // došlo k deaktivaci přiloválací návesti
        log(QString("PN: stop PN"), logging::LogLevel::Error);
        navestidlo->navestniZnak = 0;
    }

    if (r[PN]) {
        if (navestidlo) {
            navestidlo->r[TblokQ::N] = false;
            navestidlo->navestniZnak = 8;
        }
    }

    return false;
}

void TblokPN::on_tim()
{
    mtbOut[mtbOutPocitadlo].setValueBool(false);
    log(QString("PN: stop pocitadlo"), logging::LogLevel::Error);
}
