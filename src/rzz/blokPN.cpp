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
    bool pouzijTC = (tlacitkoUNavestidla != nullptr);
    bool tlacNavState = false;
    bool tlacPN = mtbIns[mtbInPN].value();

    // přečte stav vstupu


    // logika bloku PN
    if (pouzijTC) {
        tlacNavState = tlacitkoUNavestidla->mtbIns[TblokTC::mtbeIns::mtbInRuseni].value();
        r[ZF] |= tlacPN; // společním tlačítkem se nahazuje ZF

        if (r[ZF] && (tlacNavState)) r[F] = true; // pokud máme ZF a povel k PN, sepneme F
        if (r[ZF] && !(tlacPN) && !r[F]) r[ZF] = false; // pokud není F a pustíme spol. tlačítko, tak ZF odpadne
        if (!tlacNavState && r[F]) {
            r[ZF] = false;
            r[F] = false;
        }
        tlacitkoUNavestidla->r[TblokTC::rel::ZFo] = r[ZF];
    } else {
        r[F] = mtbIns[mtbInPN].value();
    }

    // detekce změny
    if (r[F] && !rLast[F]) {
        // došlo k aktivaci přivolávací návesti
        if (navestidlo != nullptr) {

            // zapni počítadlo a start časovače
            log(QString("PN: počítadlo start"), logging::LogLevel::Debug);
            mtbOut[mtbOutPocitadlo].setValueBool(true);
            tim->start(config.tPocitadlo);
        } else {
            log(QString("PN: nemáme návestidlo na PN !"), logging::LogLevel::Warning);
        }
    }

    navestidlo->r[TblokQ::rel::Fo] = r[rel::F];

    if (r != rLast) return true; else return false;
}

void TblokPN::on_tim()
{
    // časovač vypne počítadlo
    mtbOut[mtbOutPocitadlo].setValueBool(false);
    log(QString("PN: počítadlo konec"), logging::LogLevel::Debug);
}
