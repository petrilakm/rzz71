#include <iostream>
#include "rzz71.h"
#include "rzz/blokTC.h"
#include "rzz/blokPr.h"
#include "rzz/blokK.h"
#include "rzz/blokS.h"
#include "rzz/blokQ.h"
#include "rzz/blokEMZ.h"
//#include "rzz/blokQ.h"
#include "rzz/voliciskupina.h"
#include "rzz/dohledcesty.h"

bool rKPV;
bool rZ3V; // NUZ, je něco vybráno
bool rQTV; // NUZ, probíhá měření čas
bool rD3V; // NUZ, odměřeno, ruší se závěry
bool rBlik50;

TRZZ71::TRZZ71(QObject *parent)
    : QObject{parent}
{
    blik.setInterval(700);
    blik.setSingleShot(false);
    //blikOut = false;
    connect(&blik, SIGNAL(timeout()), this, SLOT(onblik()));

    tim_eval.setInterval(2000);
    tim_eval.setSingleShot(false);
    bFirstRun = true;
    connect(&tim_eval, SIGNAL(timeout()), this, SLOT(oneval()));
    t3V.setInterval(3*60*1000);
    t3V.setSingleShot(true);
    connect(&t3V, SIGNAL(timeout()), this, SLOT(ont3V()));
    t3C.setInterval(3*60*1000);
    t3C.setSingleShot(true);
    connect(&t3C, SIGNAL(timeout()), this, SLOT(ont3C()));
    t1C.setInterval(1*60*1000);
    t1C.setSingleShot(true);
    connect(&t1C, SIGNAL(timeout()), this, SLOT(ont1C()));
    t5C.setInterval(5*1000);
    t5C.setSingleShot(true);
    connect(&t5C, SIGNAL(timeout()), this, SLOT(ont5C()));

    rKPV = false;
    rZ3V = false;
    rQTV = false;
    rD3V = false;
}


void TRZZ71::readCommand(QString cmd)
{
    //
    log(QString("rzz: command \"%1\"").arg(cmd), logging::LogLevel::Debug);

    QStringList cmdList = cmd.split(' ', Qt::SkipEmptyParts);

    if (cmdList.size() > 0) {
        cmd = cmdList[0];
        if ((cmd == "help") || (cmd == "h")) {
            term(QString("b <blok> - informace o bloku"));
            term(QString("v        - informace o volící skupině"));
            term(QString("d        - informace o dohledové skupině"));
        }
        if (cmd == 'v') { // voliciskupina
            term(QString("Aktivní tlačítka = %1").arg(voliciskupina.tlacitkaAktivni.count()));
            for(TblokTC *tc : voliciskupina.tlacitkaAktivni) {
                term(QString(" - %1 -> %2 %3").arg(tc->name).arg(tc->r[TblokTC::TZ]).arg(tc->r[TblokTC::TK]));
            }
            term(QString("Postavené cesty = %1").arg(voliciskupina.cestyPostavene.count()));
            for(int ic : voliciskupina.cestyPostavene) {
                Tcesta *c = cesty->cesty[ic];
                if (c->tlacitka.count() > 1) {
                    term(QString(" - %1 -> %2 %3").arg(ic).arg(c->tlacitka[0]->name).arg(c->tlacitka[0]->name));
                }
            }
        }
        if (cmd == 'd') { // dohledcesty
            term(QString("Postavené cesty = %1").arg(dohledCesty.cestyPostavene.count()));
            for (struct TdohledCesty::cestaPodDohledem *cpd : dohledCesty.cestyPostavene) {
                term(QString(" - %1 -> %2").arg(cpd->num).arg(cpd->stav));
            }

        }
        if (cmd == 'b') { // bloky
            if (cmdList.size() > 1) {
                Tblok *b = Tblok::findBlokByName(cmdList[1]);
                if (b) {
                    switch (b->typ) {
                    case Tblok::btS:
                        term(QString("blok S"));
                        term(QString(" - J = %1").arg(b->r[TblokS::J]));
                        term(QString(" - Z = %1").arg(b->r[TblokS::Z]));
                        //term(QString(" - B = %1").arg(b->r[TblokS::B]));
                        term(QString(" - V = %1").arg(b->r[TblokS::V]));
                        break;
                    case Tblok::btK:
                        term(QString("blok K"));
                        // V, R, J, U, X1, X2, K1, K2
                        term(QString(" - J = %1").arg(b->r[TblokK::J]));
                        term(QString(" - X1 = %1").arg(b->r[TblokK::X1]));
                        term(QString(" - X2 = %1").arg(b->r[TblokK::X2]));
                        term(QString(" - K1 = %1").arg(b->r[TblokK::K1]));
                        term(QString(" - K2 = %1").arg(b->r[TblokK::K2]));
                        break;
                    case Tblok::btTC:
                        term(QString("blok TC"));
                        term(QString(" - TZ = %1").arg(b->r[TblokTC::TZ]));
                        term(QString(" - PO = %1").arg(b->r[TblokTC::PO]));
                        term(QString(" - TK = %1").arg(b->r[TblokTC::TK]));
                        break;
                    default:
                        term(QString("blok neumím vypsat"));
                        break;
                    }
                } else {
                    term(QString("blok \"%1\" nenalezen").arg(cmdList[1]));
                }
            }
        }

    }


}

void TRZZ71::init()
{
    Tblok *pBlok;
    TblokTC *pBlokTC;
    TblokPr *pBlokPr;
    TblokV *pBlokV;
    TblokV *pBlokVmaster;
    TblokS *pBlokS;
    TblokK *pBlokK;
    TblokQ *pBlokQ;
    TblokEMZ *pBlokEMZ;

    // načtě bloky ze souboru
    QStringList linelist;
    QStringList linepom;
    QStringList mtbList;
    QStringList lineparam;
    QList<uint8_t> mtbModulesUsed;
    uint8_t mtbAddr;
    uint8_t mtbPin;
    QList<mtbpin> mtbLoadInputs;
    QList<mtbpin> mtbLoadOutputs;
    QString name;
    QString type;
    QFile inputFile("bloky.csv");
    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.startsWith("#")) {
                continue;
            }
            linelist = line.split(";");
            if (linelist.count() < 6) {
                continue;
            }
            type = linelist[0];
            name = linelist[1];
            mtbLoadInputs.clear();
            mtbLoadOutputs.clear();
            // MTB  vstupy
            linepom = linelist[2].split(',');
            for (int i = 0; i < linepom.count(); i++) {
                mtbList = linepom[i].split('/');
                if (mtbList.count() == 2) {
                    mtbAddr = mtbList[0].toUInt();
                    mtbPin = mtbList[1].toUInt();
                    mtbLoadInputs.append(mtbpin(mtbAddr, mtbPin));
                    if (!mtbModulesUsed.contains(mtbAddr)) {
                        mtbModulesUsed.append(mtbAddr);
                    }
                } else {
                    mtbLoadInputs.append(mtbpin());
                }
            }
            // MTB výstupy
            linepom = linelist[3].split(',');
            for (int i = 0; i < linepom.count(); i++) {
                mtbList = linepom[i].split('/');
                if (mtbList.count() == 2) {
                    mtbAddr = mtbList[0].toUInt();
                    mtbPin = mtbList[1].toUInt();
                    mtbLoadOutputs.append(mtbpin(mtbAddr, mtbPin));
                    if (!mtbModulesUsed.contains(mtbAddr)) {
                        mtbModulesUsed.append(mtbAddr);
                    }
                } else {
                    mtbLoadOutputs.append(mtbpin());
                }
            }
            // návaznosti, parametry
            if (linelist.count() > 4) {
                lineparam = linelist[4].split(',');
            } else {
                lineparam.clear();
            }

            if (type == "KPV") {
                pinInKPV = mtbLoadInputs[0];
            }
            if (type == "NUZ") {
                pinInNUZ = mtbLoadInputs[0];
                pinOutNUZ = mtbLoadOutputs[0];
                log(QString("rzz: NUZ %1/%2, %3/%4").arg(pinInNUZ.addr).arg(pinInNUZ.pin).arg(pinOutNUZ.addr).arg(pinOutNUZ.pin), logging::LogLevel::Debug);
            }
            if (type == "Kmitac") {
                pinOutKmitac = mtbLoadOutputs[0];
            }


            if (type == "cas") {
                int casovacCas = lineparam[0].toInt() * 1000;
                if (casovacCas < 1000) casovacCas = 1000;
                log(QString("rzz: casový soubor %1 = %2 ms").arg(name).arg(casovacCas), logging::LogLevel::Debug);
                if (name == "3V") t3V.setInterval(casovacCas);
                if (name == "3C") t3C.setInterval(casovacCas);
                if (name == "1C") t1C.setInterval(casovacCas);
                if (name == "5C") t5C.setInterval(casovacCas);
            }
            if (type == "volsk") {
                voliciskupina.mtbInRuseniVolby = mtbLoadInputs[0];
                voliciskupina.mtbOutProbihaVolba = mtbLoadOutputs[0];
            }
            if (type == "TC") {
                // blokTC - tlačítko cestové
                pBlokTC = new TblokTC();
                pBlokTC->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokTC->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokTC->mtbOut[i] = mtbLoadOutputs[i];
                }
                bl.append(static_cast<Tblok*>(pBlokTC));
                log(QString("rzz: načten blok TC_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "Pr") {
                // blokPr - průsvitka
                pBlokPr = new TblokPr();
                pBlokPr->name = name;
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokPr->mtbOut[i] = mtbLoadOutputs[i];
                }
                if (lineparam.count() > 0) {
                    QString lastBlokName = lineparam[0];
                    pBlok = Tblok::findBlokByName(lastBlokName);
                    if (pBlok) {
                        pBlokPr->predBlok = pBlok;
                        if ((pBlok->typ == Tblok::btV) && (lineparam.count() > 1)) {
                            pBlokPr->predBlokMinus = (lineparam[1] == "-");
                        };
                    } else {
                        log(QString("rzz: blok %1 nemuže najít související blok \"%2\"").arg(name).arg(lastBlokName), logging::LogLevel::Error);
                    };
                }

                bl.append(static_cast<Tblok*>(pBlokPr));
                log(QString("rzz: načten blok Pr_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "V" || type == "simV") {
                // blokV - výhybka
                pBlokV = new TblokV();
                pBlokV->name = name;
                if (type == "simV") {
                    pBlokV->simulace = true;
                }
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokV->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokV->mtbOut[i] = mtbLoadOutputs[i];
                }
                if (lineparam.count() > 0) {
                    QString lastBlokName = lineparam[0];
                    pBlok = Tblok::findBlokByName(lastBlokName);
                    //if (!pBlok) log(QString("rzz: blok %1 nemuže najít související blok \"%2\"").arg(name).arg(lastBlokName), logging::LogLevel::Error);
                    if (pBlok) {
                        pBlokV->predBlok = pBlok;
                        if ((pBlok->typ == Tblok::btV) && (lineparam.count() > 1)) {
                            pBlokV->predBlokMinus = (lineparam[1] == "-");
                        }
                    };
                    if (lineparam.count() > 2) {
                        // slave blok
                        pBlokVmaster = static_cast<TblokV*>(Tblok::findBlokByName(lineparam[2]));
                        if (pBlokVmaster) {
                            pBlokV->rezimSlave = true;
                            pBlokV->dvojceBlok = pBlokVmaster;
                            pBlokVmaster->rezimMaster = true;
                            pBlokVmaster->dvojceBlok = pBlokV;
                        } else {
                            log(QString("rzz: blok %1 nemuže najít dvojici \"%2\"").arg(name).arg(lineparam[2]), logging::LogLevel::Error);
                        }
                    }
                }
                bl.append(static_cast<Tblok*>(pBlokV));
                log(QString("rzz: načten blok V_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "S" || type == "M") {
                // blokS - úsek výhýbkový i bezvýhybkový
                pBlokS = new TblokS();
                pBlokS->name = name;
                if (type == "M") pBlokS->typM = true;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokS->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokS->mtbOut[i] = mtbLoadOutputs[i];
                }
                bl.append(static_cast<Tblok*>(pBlokS));
                log(QString("rzz: načten blok S_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "K") {
                // blokK - staniční kolej
                pBlokK = new TblokK();
                pBlokK->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokK->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokK->mtbOut[i] = mtbLoadOutputs[i];
                }
                bl.append(static_cast<Tblok*>(pBlokK));
                log(QString("rzz: načten blok K_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "Q") {
                pBlokQ = new TblokQ();
                pBlokQ->name = name;
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokQ->mtbOut[i] = mtbLoadOutputs[i];
                }
                bl.append(static_cast<Tblok*>(pBlokQ));
                log(QString("rzz: načten blok Q_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "EMZ") {
                // blokEMZ - elektromagnetický zámek - simulovaný
                pBlokEMZ = new TblokEMZ();
                pBlokEMZ->name = name;

                bl.append(static_cast<Tblok*>(pBlokEMZ));
                log(QString("rzz: načten blok EMZ_%1").arg(name), logging::LogLevel::Debug);
            };
        }
    }

    cesty = new Tcesty();

    foreach (int addr, mtbModulesUsed) {
        log(QString("rzz: použitý modul %1").arg(addr), logging::LogLevel::Debug);
        emit subscribeModule(addr);
    }

    blik.start();
    tim_eval.start();
}

void TRZZ71::getInput(int addr, int pin, int state)
{
    (void) addr;
    (void) pin;
    (void) state;
    // změna na vstupu
/*
    if ((addr == 4) && (pin == 0)) {
        emit setOutput(4, 0, state);
    }
*/

}

void TRZZ71::ont3V()
{
    rD3V |= (rQTV && rZ3V);
}

void TRZZ71::ont3C()
{

}

void TRZZ71::ont1C()
{

}

void TRZZ71::ont5C()
{

}

void TRZZ71::onblik()
{
    rBlik50 = !rBlik50;
}

// proveď logiku bloků
void TRZZ71::oneval()
{
    bool changed;
    QString debug_changer = "";
    bool ret;
    // maximální počet opakování
    int i;
    if (bFirstRun) {
        bFirstRun = false;
        tim_eval.setInterval(50);
    }
    //log(QString("rzz: eval"), logging::LogLevel::Info);
    timer.start();
    // KPV
    rKPV = pinInKPV.value();
    // NUZ
    rZ3V = false; // pokud ho nikdo neaktivuje, tak aktivní není (nemá přídržný kontakt)
    for (Tblok *b : bl) {
        if (b->typ == Tblok::btS) {
            rZ3V |= b->r[TblokS::V]; // jakýkoli vybraný úsek aktivuje Z3V
        }
    }
    rD3V &= rZ3V;
    rQTV &= rZ3V;
    bool outNUZ;
    outNUZ = (rZ3V && rBlik50) || (rQTV && !rD3V); // indikace společného NUZ
    pinOutNUZ.setValueBool(outNUZ);
    if (rZ3V && !rQTV && pinInNUZ.value()) {
        // start časového souboru
        t3V.start();
        rQTV = true;
    }

    // volici skupina udělá svoje akce
    voliciskupina.evaluate();
    dohledCesty.evaluate();
    for (i = 0; i < 30; ++i) {
        changed = false;
        // pro všechny bloky
        for (Tblok *b : bl) {
            ret = b->evaluate();
            if (ret) debug_changer.append(b->name+",");
            changed |= ret;
            //changed |= b->evaluate();
        }
        if (!changed) break;
    }
    // zjístíme zda jede kmitač
    ret = false;
    for (Tblok *b : bl) {
        ret |= b->bBlikUsed;
    }
    pinOutKmitac.setValueBool(ret);

    double oneEvalTime = static_cast<double>(timer.nsecsElapsed()) * 1E-6;
    if (i>0) {
        // optimalizace pro display
        log(QString("i=%1,t=%2,%3").arg(i).arg(oneEvalTime,0, 'f', 3).arg(debug_changer), logging::LogLevel::Info);
    }

}
