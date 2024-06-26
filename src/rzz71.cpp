#include "rzz71.h"
#include "rzz/blokTC.h"
#include "rzz/blokPr.h"
#include "rzz/blokK.h"
#include "rzz/blokS.h"
#include "rzz/blokQ.h"

bool rKPV;
bool rBlik50;

TRZZ71::TRZZ71(QObject *parent)
    : QObject{parent}
{
    blik.setInterval(700);
    blik.setSingleShot(false);
    //blikOut = false;
    connect(&blik, SIGNAL(timeout()), this, SLOT(onblik()));

    tim_eval.setInterval(100);
    tim_eval.setSingleShot(false);
    connect(&tim_eval, SIGNAL(timeout()), this, SLOT(oneval()));
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
                pinKPV = mtbLoadInputs[0];
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
                log(QString("rzz: načten blok TC_%1").arg(name), logging::LogLevel::Info);
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
                log(QString("rzz: načten blok Pr_%1").arg(name), logging::LogLevel::Info);
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
                log(QString("rzz: načten blok V_%1").arg(name), logging::LogLevel::Info);
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
                log(QString("rzz: načten blok S_%1").arg(name), logging::LogLevel::Info);
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
                log(QString("rzz: načten blok K_%1").arg(name), logging::LogLevel::Info);
            }
        }
    }

    cesty = new Tcesty();

    foreach (int addr, mtbModulesUsed) {
        log(QString("rzz: použítý modul %1").arg(addr), logging::LogLevel::Debug);
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
    //log(QString("rzz: eval"), logging::LogLevel::Info);
    timer.start();
    rKPV = pinKPV.value();
    for (i = 0; i < 30; ++i) {
        changed = false;
        // pro všechny bloky
        foreach (Tblok *b, bl) {
            ret = b->evaluate();
            if (ret) debug_changer.append(b->name+", ");
            changed |= ret;
            //changed |= b->evaluate();
        }
        if (!changed) break;
    }
    double oneEvalTime = static_cast<double>(timer.nsecsElapsed()) * 1E-6;
    if (i>0) {
        log(QString("rzz: eval i=%1, time=%2 ms, %3").arg(i).arg(oneEvalTime,0, 'f', 3).arg(debug_changer), logging::LogLevel::Info);
    }

}
