#include <iostream>
#include "rzz71.h"
#include "rzz/blokTC.h"
#include "rzz/blokPr.h"
#include "rzz/blokK.h"
#include "rzz/blokS.h"
#include "rzz/blokQ.h"
#include "rzz/blokPN.h"
#include "rzz/blokEMZ.h"
#include "rzz/blokRC.h"
#include "rzz/blokTS.h"
#include "rzz/blokOs.h"
#include "rzz/blokKU.h"
#include "rzz/voliciskupina.h"
#include "rzz/dohledcesty.h"

bool rKPV;
bool rDCCZkrat;
bool rDCCVypadek;
bool rZ3V; // NUZ, je něco vybráno
bool rQTV; // NUZ, probíhá měření času
bool rD3V; // NUZ, odměřeno, ruší se závěry
bool rZ5C, rD5C;
bool rZ1C, rD1C;
bool rZ3C, rD3C;
bool rBlik50;
bool rBlik100;
bool rNavNoc;

struct sconfig config;


TRZZ71::TRZZ71(QObject *parent)
    : QObject{parent}
{
    tcpcon = new Ttcpconsole;

    connect(&logger, SIGNAL(logEvent(QString,logging::LogLevel)), tcpcon, SLOT(sendEvent(QString,logging::LogLevel)));
    connect(tcpcon, SIGNAL(newLine(QString)), this, SLOT(readCommand(QString)));

    blik.setInterval(350);
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
    rDCCZkrat = false;
    rDCCVypadek = false;
    rZ3V = false;
    rQTV = false;
    rD3V = false;
    rZ5C = rD5C =false;
    rZ1C = rD1C =false;
    rZ3C = rD3C =false;
    rNavNoc = false;

    config.tOdpadN = 300;
    config.tPocitadlo = 400;
    config.tSimulacePrest = 2000;
    config.tUvolneniZ = 300;
    config.tZvonek = 800;
    config.cfgVybav = false;
    config.cfgSimulV = false;

    simul_puls_timer.setInterval(50);
    simul_puls_timer.setSingleShot(true);
    connect(&simul_puls_timer, SIGNAL(timeout()), this, SLOT(onSimulPuls()));

}


void TRZZ71::readCommand(QString cmd)
{
    TblokV *pBlokV;
    TblokRC *pBlokRC;
    QString tmp; // pro sestavování stringu
    //
    log(QString("rzz: command \"%1\"").arg(cmd), logging::LogLevel::Debug);

    QStringList cmdList = cmd.split(' ', Qt::SkipEmptyParts);

    if (cmdList.size() > 0) {
        cmd = cmdList[0];
        if ((cmd == "help") || (cmd == "h")) {
            tcpcon->sendMsg(QString("l        - seznam bloků"));
            tcpcon->sendMsg(QString("b <blok> - informace o bloku"));
            tcpcon->sendMsg(QString("v        - informace o volící skupině"));
            tcpcon->sendMsg(QString("d        - informace o dohledové skupině"));
            tcpcon->sendMsg(QString("m <addr> <pin> <stav> - simulace změny vstupu"));
            tcpcon->sendMsg(QString("M <addr> <pin> - simulace impulsu na vstupu"));
            tcpcon->sendMsg(QString(""));
            tcpcon->sendMsg(QString("přikazy pro konzoli:"));
            tcpcon->sendMsg(QString(".    (tečka) - opakuj poslední příkaz"));
            tcpcon->sendMsg(QString("loglevel     - vypiš aktuální loglevel"));
            tcpcon->sendMsg(QString("loglevel <n> - nastav loglevel na <n> (0-6)"));
            tcpcon->sendMsg(QString("exit         - odpojit klienta"));
            tcpcon->sendMsg(QString("list         - seznam připojených klientů"));
        }
        if (cmd == 'm') { // mtb simulace
            if (cmdList.length() < 4) {
                tcpcon->sendMsg(QString("m <addr> <pin> <stav>"));
            } else {
                mtbpin p(cmdList[1].toInt(), cmdList[2].toInt());
                mtb.module[p.addr].inputs[p.pin] = cmdList[3].toInt();
            }
        }
        if (cmd == 'M') { // mtb simulace - impuls
            if (cmdList.length() < 3) {
                tcpcon->sendMsg(QString("M <addr> <pin>"));
            } else {
                mtbpin p(cmdList[1].toInt(), cmdList[2].toInt());
                simul_puls_pin = p;
                log(QString("addr=%1, pin=%2").arg(p.addr).arg(p.pin), logging::LogLevel::Debug);
                mtb.module[p.addr].inputs[p.pin] = 1;
                simul_puls_timer.start();
            }
        }
        if (cmd == 'v') { // voliciskupina
            tcpcon->sendMsg(QString("Aktivní tlačítka = %1").arg(voliciskupina.tlacitkaAktivni.count()));
            for(TblokTC *tc : voliciskupina.tlacitkaAktivni) {
                tcpcon->sendMsg(QString(" - %1 -> TZ=%2 VA=%3").arg(tc->name).arg(tc->r[TblokTC::TZ]).arg(tc->r[TblokTC::VA]));
            }
        }
        if (cmd == 'd') { // dohledcesty
            tcpcon->sendMsg(QString("Postavené cesty = %1").arg(dohledCesty.cestyPostavene.count()));
            for (TdohledCesty::cestaPodDohledem *cpd : dohledCesty.cestyPostavene) {
                tcpcon->sendMsg(QString(" - %1 -> stav %2-%3").arg(cpd->num).arg(cpd->stav).arg(dohledCesty.stavCesty2QString(cpd->stav)));
                tcpcon->sendMsg(QString("   EV = %1, RC = %2").arg(cpd->vlakEV).arg(cpd->ruseni));
                tcpcon->sendMsg(QString("   vlak v bloku %1 až %2").arg(cpd->vlakCelo).arg(cpd->vlakKonec));
                for (QString upo1 : cpd->upo) {
                    tcpcon->sendMsg(tr("   UPO - %1").arg(upo1));
                }
            }
        }
        if (cmd == 'l') { // seznam bloků
            for (Tblok *b : bl) {
                tcpcon->sendMsg(QString(" - %1").arg(b->name));
            }
        }
        if (cmd == 'b') { // bloky
            if (cmdList.size() > 1) {
                Tblok *b = Tblok::findBlokByName(cmdList[1]);
                if (b) {
                    switch (b->typ) {
                    case Tblok::btV:
                        tcpcon->sendMsg(QString("blok V"));
                        pBlokV = static_cast<TblokV*>(b);
                        tcpcon->sendMsg(QString(" - master = %1").arg(pBlokV->rezimMaster));
                        tcpcon->sendMsg(QString(" - slave  = %1").arg(pBlokV->rezimSlave));
                        if (pBlokV->dvojceBlok) tcpcon->sendMsg(QString(" -- dvojceBlok = %1").arg(pBlokV->dvojceBlok->name));
                        if (pBlokV->predBlok) tcpcon->sendMsg(QString(" -- predBlok = %1").arg(pBlokV->predBlok->name));
                        if (pBlokV->predBlok) tcpcon->sendMsg(QString(" -- predBlok směr mínus = %1").arg(pBlokV->predBlokMinus));
                        if (pBlokV->odvratneBloky.count() > 0) {
                            tcpcon->sendMsg(QString(" - odvrané bloky:"));
                            for (Tblok *odvBlok : pBlokV->odvratneBloky) {
                                tcpcon->sendMsg(QString("   - %1").arg(odvBlok->name));
                            }
                        }
                        tcpcon->sendMsg(QString(" - J   = %1").arg(b->r[TblokV::J]));
                        tcpcon->sendMsg(QString(" - Z   = %1").arg(b->r[TblokV::Z]));
                        tcpcon->sendMsg(QString(" - ZP  = %1").arg(b->r[TblokV::ZP]));
                        tcpcon->sendMsg(QString(" - BP  = %1").arg(b->r[TblokV::BP]));
                        tcpcon->sendMsg(QString(" - DP  = %1").arg(b->r[TblokV::DP]));
                        tcpcon->sendMsg(QString(" - DM  = %1").arg(b->r[TblokV::DM]));
                        tcpcon->sendMsg(QString(" - VOP = %1").arg(b->r[TblokV::VOP]));
                        tcpcon->sendMsg(QString(" - VOM = %1").arg(b->r[TblokV::VOM]));
                        tcpcon->sendMsg(QString(" - SP  = %1").arg(b->r[TblokV::SP]));
                        tcpcon->sendMsg(QString(" - SM  = %1").arg(b->r[TblokV::SM]));
                        tcpcon->sendMsg(QString(" - INe = %1").arg(b->r[TblokV::INe]));
                        break;
                    case Tblok::btEMZ:
                        tcpcon->sendMsg(QString("blok EMZ"));
                        tcpcon->sendMsg(QString(" - UK = %1").arg(b->r[TblokEMZ::UK]));
                        tcpcon->sendMsg(QString(" - ZP = %1").arg(b->r[TblokEMZ::ZP]));
                        tcpcon->sendMsg(QString(" - Z  = %1").arg(b->r[TblokEMZ::Z]));
                        break;
                    case Tblok::btS:
                        tcpcon->sendMsg(QString("blok S"));
                        tcpcon->sendMsg(QString(" - J  = %1").arg(b->r[TblokS::rel::J]));
                        tcpcon->sendMsg(QString(" - Z  = %1").arg(b->r[TblokS::rel::Z]));
                        //tcpcon->sendMsg(QString(" - B  = %1").arg(b->r[TblokS::B]));
                        tcpcon->sendMsg(QString(" - V  = %1").arg(b->r[TblokS::rel::V]));
                        break;
                    case Tblok::btK:
                        tcpcon->sendMsg(QString("blok K"));
                        // V, R, J, U, X1, X2, K1, K2
                        tcpcon->sendMsg(QString(" - J  = %1").arg(b->r[TblokK::J]));
                        tcpcon->sendMsg(QString(" - X1 = %1").arg(b->r[TblokK::X1]));
                        tcpcon->sendMsg(QString(" - X2 = %1").arg(b->r[TblokK::X2]));
                        tcpcon->sendMsg(QString(" - K1 = %1").arg(b->r[TblokK::K1]));
                        tcpcon->sendMsg(QString(" - K2 = %1").arg(b->r[TblokK::K2]));
                        break;
                    case Tblok::btTC:
                        tcpcon->sendMsg(QString("blok TC"));
                        tcpcon->sendMsg(QString(" - TZ = %1").arg(b->r[TblokTC::rel::TZ]));
                        tcpcon->sendMsg(QString(" - PO = %1").arg(b->r[TblokTC::rel::PO]));
                        tcpcon->sendMsg(QString(" - VA = %1").arg(b->r[TblokTC::rel::VA]));
                        tcpcon->sendMsg(QString(" - RC = %1").arg(b->r[TblokTC::rel::RC]));
                        tcpcon->sendMsg(QString(" - NM = %1").arg(b->r[TblokTC::rel::NM]));
                        tcpcon->sendMsg(QString(" - ZFo= %1").arg(b->r[TblokTC::rel::ZFo]));
                        break;
                    case Tblok::btQ:
                        tcpcon->sendMsg(QString("blok Q"));
                        tcpcon->sendMsg(QString(" - Fo = %1").arg(b->r[TblokQ::rel::Fo]));
                        tcpcon->sendMsg(QString(" - N  = %1").arg(b->r[TblokQ::rel::N]));
                        tcpcon->sendMsg(QString(" - Nv = %1").arg(b->r[TblokQ::rel::Nv]));
                        tcpcon->sendMsg(QString(" - kód požada. = %1").arg(static_cast<TblokQ*>(b)->navestniZnak));
                        tcpcon->sendMsg(QString(" - kód návěsti = %1").arg(static_cast<TblokQ*>(b)->navestniZnakReal));
                        break;
                    case Tblok::btPN:
                        tcpcon->sendMsg(QString("blok PN"));
                        tcpcon->sendMsg(QString(" - ZF = %1").arg(b->r[TblokPN::rel::ZF]));
                        tcpcon->sendMsg(QString(" - F  = %1").arg(b->r[TblokPN::rel::F]));
                        if (static_cast<TblokPN*>(b)->navestidlo != nullptr) {
                            tcpcon->sendMsg(QString(" -- návěstidlo = %1").arg(static_cast<TblokPN*>(b)->navestidlo->name));
                        }
                        if (static_cast<TblokPN*>(b)->tlacitkoUNavestidla != nullptr) {
                            tcpcon->sendMsg(QString(" -- tlačítko = %1").arg(static_cast<TblokPN*>(b)->tlacitkoUNavestidla->name));
                        }
                        break;
                    case Tblok::btOs:
                        tcpcon->sendMsg(QString("blok Os"));
                        tcpcon->sendMsg(QString(" - OSV= %1").arg(b->r[TblokOs::rel::OSV]));
                        break;
                    case Tblok::btRC:
                        pBlokRC = static_cast<TblokRC*>(b);
                        tcpcon->sendMsg(QString("blok RC"));
                        tcpcon->sendMsg(QString(" - RC = %1").arg(b->r[TblokRC::rel::EV]));
                        tmp = "";
                        for(int i = 0; i < pBlokRC->cestyRC.count(); i++) {
                            tmp.append(QString("%1,").arg(pBlokRC->cestyRC[i]));
                        }
                        tcpcon->sendMsg(QString(" - cesty = %1").arg(tmp));
                        break;
                    case Tblok::btPr:
                        tcpcon->sendMsg(QString("blok Pr"));
                        tcpcon->sendMsg(QString(" - bílá    = %1").arg(b->r[TblokPr::rel::PrB]));
                        tcpcon->sendMsg(QString(" - červená = %1").arg(b->r[TblokPr::rel::PrC]));
                        if (static_cast<TblokPr*>(b)->predBlok != nullptr) {
                            tcpcon->sendMsg(QString(" -- předblok = %1").arg(static_cast<TblokPr*>(b)->predBlok->name));
                            tcpcon->sendMsg(QString(" -- předblok směr mínus = %1").arg(static_cast<TblokPr*>(b)->predBlokMinus));
                        }
                        break;
                    case Tblok::btKU:
                        tcpcon->sendMsg(QString("blok KU"));
                        tcpcon->sendMsg(QString(" - J  = %1").arg(b->r[TblokKU::rel::J]));
                        tcpcon->sendMsg(QString(" - ZP = %1").arg(b->r[TblokKU::rel::ZP]));
                        tcpcon->sendMsg(QString(" - OC = %1").arg(b->r[TblokKU::rel::OC]));
                        tcpcon->sendMsg(QString(" - OCP= %1").arg(b->r[TblokKU::rel::OCP]));
                        tcpcon->sendMsg(QString(" - EVO= %1").arg(b->r[TblokKU::rel::EVO]));
                        break;
                    default:
                        tcpcon->sendMsg(QString("blok neumím vypsat"));
                        break;
                    }
                } else {
                    tcpcon->sendMsg(QString("blok \"%1\" nenalezen").arg(cmdList[1]));
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
    TblokPN *pBlokPN;
    TblokEMZ *pBlokEMZ;
    TblokRC *pBlokRC;
    TblokTS *pBlokTS;
    TblokOs *pBlokOs;
    TblokKU *pBlokKU;

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
    // načti definiční soubor
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
            if (type == "zkrat") {
                pinInDCCZkrat = mtbLoadInputs[0];
                pinInDCCVypadek = mtbLoadInputs[1];
            }
            if (type == "KPV") {
                pinInKPV = mtbLoadInputs[0];
            }
            if (type == "NavNoc") {
                pinInNavNoc = mtbLoadInputs[0];
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
                int casovacCas = lineparam[0].toInt();
                int casovacCasLim = casovacCas;
                if (casovacCas < 100) casovacCasLim = 100;
                log(QString("rzz: casový soubor %1 = %2 ms").arg(name).arg(casovacCas), logging::LogLevel::Debug);
                if (name == "3V") {
                    t3V.setInterval(casovacCasLim);

                }
                if (name == "3C") {
                    t3C.setInterval(casovacCasLim);
                    pinInd3C = mtbLoadOutputs[0];
                }
                if (name == "1C") {
                    t1C.setInterval(casovacCasLim);
                    pinInd1C = mtbLoadOutputs[0];
                }
                if (name == "5C") {
                    t5C.setInterval(casovacCasLim);
                    pinInd5C = mtbLoadOutputs[0];
                }
                if (name == "pocitadlo") config.tPocitadlo = casovacCasLim;
                if (name == "uvolneniZ") config.tUvolneniZ = casovacCasLim;
                if (name == "odpadN") config.tOdpadN = casovacCasLim;
                if (name == "prestavI") config.tProudPrestavniku = casovacCas;
                if (name == "zvonek") config.tZvonek = casovacCasLim;
            }
            if (type == "volsk") {
                voliciskupina.mtbInRuseniVolby = mtbLoadInputs[0];
                voliciskupina.mtbOutProbihaVolba = mtbLoadOutputs[0];
            }
            if (type == "TC") {
                // blokTC - tlačítko cestové
                pBlokTC = new TblokTC(this);
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
                pBlokPr = new TblokPr(this);
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
                pBlokV = new TblokV(this);
                pBlokV->name = name;
                //if (type == "simV") {
                //    pBlokV->simulace = true;
                //}
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokV->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokV->mtbOut[i] = mtbLoadOutputs[i];
                }
                if (lineparam.count() > 0) {
                    QString lastBlokName = lineparam[0];
                    if ((lastBlokName != "") && (lastBlokName != "_")) {
                        pBlok = Tblok::findBlokByName(lastBlokName);
                        if (!pBlok) log(QString("rzz: blok %1 nemuže najít související blok \"%2\"").arg(name).arg(lastBlokName), logging::LogLevel::Error);
                        if (pBlok) {
                            pBlokV->predBlok = pBlok;
                            if ((pBlok->typ == Tblok::btV) && (lineparam.count() > 1)) {
                                pBlokV->predBlokMinus = (lineparam[1] == "-");
                            }
                        };
                    }
                    if (lineparam.count() > 2) {
                        // slave blok
                        pBlokVmaster = static_cast<TblokV*>(Tblok::findBlokByName(lineparam[2]));
                        if (pBlokVmaster) {
                            pBlokV->rezimSlave = true;
                            pBlokV->dvojceBlok = pBlokVmaster;
                            pBlokVmaster->rezimMaster = true;
                            pBlokVmaster->dvojceBlok = pBlokV;
                        } else {
                            if ((lineparam[2] != "") && (lineparam[2] != "_")) {
                                log(QString("rzz: blok %1 nemuže najít dvojici \"%2\"").arg(name).arg(lineparam[2]), logging::LogLevel::Error);
                            }
                        }
                    }
                }
                bl.append(static_cast<Tblok*>(pBlokV));
                log(QString("rzz: načten blok V_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "S" || type == "M") {
                // blokS - úsek výhýbkový i bezvýhybkový
                pBlokS = new TblokS(this);
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
                pBlokK = new TblokK(this);
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
            if (type == "PN") {
                pBlokPN = new TblokPN(this);
                pBlokPN->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokPN->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokPN->mtbOut[i] = mtbLoadOutputs[i];
                }
                if (lineparam.count() > 0) {
                    QString navestidloName = lineparam[0];
                    pBlok = Tblok::findBlokByName(navestidloName);
                    if (pBlok) {
                        if (pBlok->typ == Tblok::btQ) {
                            pBlokPN->navestidlo = static_cast<TblokQ*>(pBlok);
                        } else {
                            log(QString("rzz: blok PN_%1 nemuže najít související návestidlo, \"%2\" není návěstidlo").arg(name).arg(navestidloName), logging::LogLevel::Error);
                        }
                    } else {
                        log(QString("rzz: blok PN_%1 nemuže najít související návestidlo \"%2\"").arg(name).arg(navestidloName), logging::LogLevel::Error);
                    };
                }
                if (lineparam.count() > 1) {
                    QString tlacitkoName = lineparam[1];
                    pBlok = Tblok::findBlokByName(tlacitkoName);
                    if (pBlok) {
                        if (pBlok->typ == Tblok::btTC) {
                            pBlokPN->tlacitkoUNavestidla = static_cast<TblokTC*>(pBlok);
                        } else {
                            log(QString("rzz: blok PN_%1 nemuže najít související tlačítko, \"%2\" není tlačítko").arg(name).arg(tlacitkoName), logging::LogLevel::Error);
                        }
                    } else {
                        log(QString("rzz: blok PN_%1 nemuže najít související tlačítko \"%2\"").arg(name).arg(tlacitkoName), logging::LogLevel::Error);
                    };
                }
                bl.append(static_cast<Tblok*>(pBlokPN));
                log(QString("rzz: načten blok PN_%1").arg(name), logging::LogLevel::Debug);
            }
            if (type == "EMZ") {
                // blokEMZ - elektromagnetický zámek - simulovaný
                pBlokEMZ = new TblokEMZ(this);
                pBlokEMZ->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokEMZ->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokEMZ->mtbOut[i] = mtbLoadOutputs[i];
                }
                // načtení, které výmeny zámek zamyká
                for (int i = 0; i < lineparam.count(); i++) {
                    QString vymenaName = lineparam[i];
                    pBlok = Tblok::findBlokByName(vymenaName);
                    if (!pBlok) log(QString("rzz: blok %1 nemuže najít blok pro EMZ \"%2\"").arg(name).arg(vymenaName), logging::LogLevel::Error);
                    if (pBlok) {
                        if (pBlok->typ == Tblok::btV) {
                            pBlokEMZ->vym.append(static_cast<TblokV*>(pBlok));
                        }
                    }
                }
                bl.append(static_cast<Tblok*>(pBlokEMZ));
                log(QString("rzz: načten blok EMZ_%1").arg(name), logging::LogLevel::Debug);
            };
            if (type == "RC") {
                // blokRC - tlačítko na rušení cesty pro projetí vlakem (TEST)
                pBlokRC = new TblokRC(this);
                pBlokRC->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokRC->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokRC->mtbOut[i] = mtbLoadOutputs[i];
                }
                for(QString &s : lineparam){
                    if (s.contains('-')) {
                        QStringList cestyRozsah = s.split('-', Qt::KeepEmptyParts);
                        if (cestyRozsah.count() == 2) {
                            int RCmin = cestyRozsah[0].toInt();
                            int RCmax = cestyRozsah[1].toInt();
                            for (int i = RCmin; i <= RCmax; i++) {
                                pBlokRC->cestyRC.append(i);
                            }
                        } else {
                            log(QString("rzz: blok RC_%1 má neplatný rozsah cest \"%2\"").arg(name).arg(s), logging::LogLevel::Error);
                        }
                    } else {
                        pBlokRC->cestyRC.append(s.toInt());
                    }
                }
                log(QString("rzz: načten blok RC_%1").arg(name), logging::LogLevel::Debug);
                bl.append(static_cast<Tblok*>(pBlokRC));
            }
            if (type == "TS") {
                // blokTS - traťový souhlas
                pBlokTS = new TblokTS(this);
                pBlokTS->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokTS->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokTS->mtbOut[i] = mtbLoadOutputs[i];
                }
                log(QString("rzz: načten blok TS_%1").arg(name), logging::LogLevel::Debug);
                bl.append(static_cast<Tblok*>(pBlokTS));
            }
            if (type == "Os") {
                // blokOs - Osvetlení nebo obecný vstup/výstup
                pBlokOs = new TblokOs(this);
                pBlokOs->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokOs->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokOs->mtbOut[i] = mtbLoadOutputs[i];
                }
                log(QString("rzz: načten blok Os_%1").arg(name), logging::LogLevel::Debug);
                bl.append(static_cast<Tblok*>(pBlokOs));
            }
            if (type == "KU") {
                pBlokKU = new TblokKU(this);
                pBlokKU->name = name;
                for (int i = 0; i < mtbLoadInputs.count(); i++) {
                    pBlokKU->mtbIns[i] = mtbLoadInputs[i];
                }
                for (int i = 0; i < mtbLoadOutputs.count(); i++) {
                    pBlokKU->mtbOut[i] = mtbLoadOutputs[i];
                }
                log(QString("rzz: načten blok KU_%1").arg(name), logging::LogLevel::Debug);
                bl.append(static_cast<Tblok*>(pBlokKU));
            }
            if (type == "config") {
                if (mtbLoadInputs.count() >= 4) {
                    pinConfig1 = mtbLoadInputs[0];
                    pinConfig2 = mtbLoadInputs[1];
                    pinConfig3 = mtbLoadInputs[2];
                    pinConfig4 = mtbLoadInputs[3];
                }
            }
        }
    }

    // ampérmetr - hardcoded, ToDo: do souboru !
    pinAmp01 = mtbpin(19,16);
    pinAmp02 = mtbpin(19,17);
    pinAmp04 = mtbpin(19,18);
    pinAmp08 = mtbpin(19,19);
    pinAmp10 = mtbpin(19,20);
    pinAmp20 = mtbpin(19,21);

    // virtuální blok - návestidlo stále na volno
    pBlokQ = new TblokQ();
    pBlokQ->name = "T";
    pBlokQ->navestniZnak = 1;
    pBlokQ->r[TblokQ::rel::N] = true;
    pBlokQ->r[TblokQ::rel::N] = true;
    bl.append(static_cast<Tblok*>(pBlokQ));
    log(QString("rzz: vytvořen virtuální blok Q_T"), logging::LogLevel::Debug);

    // načte cesty (vlakové i posunové)
    cesty = new Tcesty();

    // přihlásí se k odběru zpráv z MTB modulů
    foreach (int addr, mtbModulesUsed) {
        log(QString("rzz: použitý modul %1").arg(addr), logging::LogLevel::Debug);
        emit subscribeModule(addr);
    }

    // start časovačů
    blik.start();
    tim_eval.start();
}

void TRZZ71::onSimulPuls()
{
    mtb.module[simul_puls_pin.addr].inputs[simul_puls_pin.pin] = 0;
}

void TRZZ71::getInput(int addr, int pin, int state)
{
    (void) addr;
    (void) pin;
    (void) state;

    log(QString("rzz: in %1, pin %2, state %3")
            .arg(addr)
            .arg(pin)
            .arg(state)
        , logging::LogLevel::Debug);
    // změna na vstupu

}

void TRZZ71::ont3V()
{
    rD3V |= (rQTV && rZ3V);
}

void TRZZ71::ont3C()
{
    rD3C |= (rZ3C);
}

void TRZZ71::ont1C()
{
    rD1C |= (rZ1C);
}

void TRZZ71::ont5C()
{
    rD5C |= (rZ5C);
}

void TRZZ71::onblik()
{
    rBlik100 = !rBlik100;
    if (rBlik100) rBlik50 ^= 1;
}

// proveď logiku bloků
void TRZZ71::oneval()
{
    bool changed;
    QString debug_changer = "";
    bool ret;
    int prestavnyProud;
    // maximální počet opakování
    int i;
    if (bFirstRun) {
        bFirstRun = false;
        tim_eval.setInterval(50);
    }
    //log(QString("rzz: eval"), logging::LogLevel::Info);
    timer.start();
    // zkrat zesilovače - výpadek DCC
    rDCCZkrat = pinInDCCZkrat.value();
    rDCCVypadek = pinInDCCVypadek.value();
    // KPV
    rKPV = pinInKPV.value();
    // NavNoc
    rNavNoc = pinInNavNoc.value();
    // NUZ - časovač 3V
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
    // časovače RC
    rZ5C = false; // pokud ho nikdo neaktivuje, tak aktivní není (nemá přídržný kontakt)
    rZ1C = false;
    rZ3C = false;

    // volici skupina udělá svoje akce
    voliciskupina.evaluate();
    dohledCesty.evaluate();
    // bloky provedou svoje akce
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

    // vyřešíme časovače RC
    rD5C &= rZ5C;
    rD1C &= rZ1C;
    rD3C &= rZ3C;
    // indikace na panel
    bool out5C;
    bool out1C;
    bool out3C;
    out5C = ((rZ5C && !rD5C) || (rBlik50 && (rD5C && rZ5C))); // indikace časovače 5C
    out1C = ((rZ1C && !rD1C) || (rBlik50 && (rD5C && rZ5C))); // indikace časovače 1C
    out3C = ((rZ3C && !rD3C) || (rBlik50 && (rD5C && rZ5C))); // indikace časovače 3C
    pinInd5C.setValueBool(out5C);
    pinInd1C.setValueBool(out1C);
    pinInd3C.setValueBool(out3C);
    // zapíná a vypíná časovače
    if ( rZ5C && !t5C.isActive()) t5C.start();
    if (!rZ5C &&  t5C.isActive()) t5C.stop();
    if ( rZ1C && !t1C.isActive()) t1C.start();
    if (!rZ1C &&  t1C.isActive()) t1C.stop();
    if ( rZ3C && !t3C.isActive()) t3C.start();
    if (!rZ3C &&  t3C.isActive()) t3C.stop();


    // čte nastavení
    config.cfgVybav = pinConfig1.value();
    config.cfgSimulV = pinConfig2.value();

    //spočítíme přestavný proud
    prestavnyProud = 0;
    for (Tblok *b : bl) {
        if (b->typ == Tblok::btV) {
            prestavnyProud += static_cast<TblokV*>(b)->prestavnyProud;
        }
    }
    if (prestavnyProud > 63) prestavnyProud = 63; // limitace maxima
    // a dáme jej na výstup
    pinAmp01.setValueBool(((prestavnyProud >> 0) & 1));
    pinAmp02.setValueBool(((prestavnyProud >> 1) & 1));
    pinAmp04.setValueBool(((prestavnyProud >> 2) & 1));
    pinAmp08.setValueBool(((prestavnyProud >> 3) & 1));
    pinAmp10.setValueBool(((prestavnyProud >> 4) & 1));
    pinAmp20.setValueBool(((prestavnyProud >> 5) & 1));

    //double oneEvalTime = static_cast<double>(timer.nsecsElapsed()) * 1E-6; // ms
    //if (i>0) {
        // optimalizace pro display
        //log(QString("i=%1,t=%2,%3").arg(i).arg(oneEvalTime,0, 'f', 3).arg(debug_changer), logging::LogLevel::Info);
    //}

}
