#include "blokQ.h"

TblokQ::TblokQ() {
    typ = btQ;
    for (int i = 0; i < RELAY_COUNT_Q; ++i) {
        r.append(false);
    }
    navestniZnak = 0;
    navestniZnakReal = 0;
    relLastFo = false;
    tim = new QTimer(this);
    tim->setSingleShot(true);
    tim->setInterval(config.tOdpadN);
    connect(tim, SIGNAL(timeout()), this, SLOT(onTimTimeout()));
}

bool TblokQ::evaluate()
{
    QList<bool> rLast = r;
    // vstupy:
    // relé N a proměnná navestniZnak

    // logika

    // zpoždění 2s odpadení relé N (pro rušení i nesplnění podmínek)
    if (!r[N] && r[Nreal] && !tim->isActive()) {
        tim->start();
        log(QString("Q: odpad relé N, začátek měření času na návěstidle %1").arg(this->name), logging::LogLevel::Commands);
    }
    if (r[N] && tim->isActive()) {
        tim->stop();
        log(QString("Q: odpad relé N, konec měření času, relé stále přitaženo na návestidle %1").arg(this->name), logging::LogLevel::Commands);
    }

    // co bude na skutečném návestidle za znak
    if (r[Fo]) {
        navestniZnakReal = 8;
        r[N] = false;
    } else {
        if (r[N]) r[Nreal] = true; // N zapíná hned
        if (r[Nreal]) {
            navestniZnakReal = navestniZnak; // zobrazí požadovaný návestní znak
        } else {
            navestniZnakReal = 0;
        }
    }

    bool bHZ,bZ,bC,bB,bDN;
    bool bBl  = false; // kmitání na návestidle
    bool bBl2 = false; // kmitání na předvěsti

    bHZ=bZ=bC=bB=bDN=0;

    switch (navestniZnakReal) {
    case 0:  bC=1; break; // stuj
    case 1:  bZ=1; bDN=1; break; // volno
    case 2:  bHZ=1; bDN=1; break; // výstraha
    case 3:  bHZ=1; bDN=1; bBl=1; break; // očekávej 40
    case 4:  bZ=1; bDN=1; bBl2=1; break; // 40 + volno
    case 6:  bHZ=1; bDN=1; bBl2=1; break; // 40 + výstraha
    case 7:  bHZ=1; bDN=1; bBl=1; bBl2=1; break; // 40 + očekávej 40
    case 8:  bC=1; bB=rBlik50; bBl=1; break; // přivolávačka
    case 9:  bB=1; break; // posun dovolen
    }
    bBlikUsed=(bBl);
    if (mtbOut[mtbOutMakCervena].valid) {
        // vjezdové návestidlo, má použitou červenou, má kmitavé předvěsti
        bBlikUsed |=(bBl2);
    }
    // výstupy
    mtbOut[mtbOutMakHorniZluta].setValueBool(bHZ);
    mtbOut[mtbOutMakZelena].setValueBool(bZ);
    mtbOut[mtbOutMakCervena].setValueBool(bC);
    mtbOut[mtbOutMakBila].setValueBool(bB);
    mtbOut[mtbOutMakDNVC].setValueBool(bDN);
    mtbOut[mtbOutScom].setValueScom(navestniZnakReal);

    if (r != rLast) return true; else return false;
    r[N] = r[Nreal]; // pokud návestní relé neodpadlo, chová se jako přitažené
}

void TblokQ::onTimTimeout()
{
    if (r[rel::N] == false) {
        r[rel::Nreal] = false;
        navestniZnakReal = 0;
        log(QString("Q: odpad relé N po zpoždění na návěstidle %1").arg(this->name), logging::LogLevel::Commands);
    }
}
