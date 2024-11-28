#include "blokPN.h"

TblokPN::TblokPN() {
    typ = btPN;
    name="-";
    for (int i = 0; i < RELAY_COUNT_PN; ++i) {
        r.append(false);
    }
    // timer pro puls pro počítadlo
    tim = new QTimer(this);
    tim->setSingleShot(true);
    connect(tim, &QTimer::timeout, this,  &TblokPN::on_tim);
    tlacitkoUNavestidla = nullptr;
    navestidlo = nullptr;
}

bool TblokPN::evaluate()
{
    QList<bool> rLast = r;
    // logika
    bool povelkPN = false;
    bool pouzijTC = (tlacitkoUNavestidla != nullptr);

    // přečte stav vstupu
    povelkPN = mtbIns[mtbInPN].value();

    // logika bloku PN
    if (pouzijTC) {
        tlacitkoUNavestidla->r[TblokTC::rel::BR] = povelkPN;
        if (povelkPN) {
            r[rel::PN] = tlacitkoUNavestidla->mtbIns[TblokTC::mtbeIns::mtbInRuseni].value();
        } else {
            r[rel::PN] = false;
        }
    } else {
        r[rel::PN] = povelkPN;
    }

    // detekce změny
    if (r[PN] && !rLast[PN]) {
        // došlo k aktivaci přiloválací návesti
        if (navestidlo != nullptr) {
            log(QString("PN: zapnout PN na návěstidle %1").arg(navestidlo->name), logging::LogLevel::Commands);
            // zapni počítadlo a starti časovače
            log(QString("PN: počítadlo start"), logging::LogLevel::Debug);
            mtbOut[mtbOutPocitadlo].setValueBool(true);
            tim->start(800);
        } else {
            log(QString("PN: nemáme návestidlo na PN !"), logging::LogLevel::Warning);
        }
    }

    if (!r[PN] && rLast[PN]) {
        // došlo k deaktivaci přiloválací návesti

        if (navestidlo != nullptr) {
            log(QString("PN: vypnout PN na návěstidle %1").arg(navestidlo->name), logging::LogLevel::Commands);
            navestidlo->navestniZnak = 0;
        }
    }

    if (r[PN]) {
        if (navestidlo != nullptr) {
            navestidlo->r[TblokQ::N] = false;
            navestidlo->navestniZnak = 8;
        }
    }

    if (r != rLast) return true; else return false;
}

void TblokPN::on_tim()
{
    // časovač vypne počítadlo
    mtbOut[mtbOutPocitadlo].setValueBool(false);
    log(QString("PN: počítadlo konec"), logging::LogLevel::Debug);
}
