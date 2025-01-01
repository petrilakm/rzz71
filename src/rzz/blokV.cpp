#include "blokV.h"

TblokV::TblokV(QObject *parent) : Tblok{parent} {
    typ = btV;
    predBlok = NULL;
    dvojceBlok = NULL;
    odvratneBloky.clear();
    rezimMaster = false;
    rezimSlave = false;
    simulace = false;
    for (int i = 0; i < RELAY_COUNT_V; ++i) {
        r.append(false);
    }
    simulace = false;
    simulacePoloha = 0;
    prestavnyProud = 0;
    tim = new QTimer();
    tim->setInterval(config.tSimulacePrest);
    tim->setSingleShot(true);
    connect(tim, SIGNAL(timeout()), this, SLOT(onTimTimeout()));
    tim->start();
}

bool TblokV::evaluate()
{
    QList<bool> rLast = r;
    bBlikUsed = false;
    //
    simulace = config.cfgSimulV;
    // logika
    // vstupy z MTB
    if (rezimSlave) {
        if (simulace) {
            r[KP] = (simulacePoloha == 1) && (dvojceBlok->simulacePoloha == 1);
            r[KM] = (simulacePoloha == 2) && (dvojceBlok->simulacePoloha == 2);
        } else {
            r[KP] = mtbIns[mtbInKontrolaPlus].value() && dvojceBlok->mtbIns[mtbInKontrolaPlus].value();
            r[KM] = mtbIns[mtbInKontrolaMinus].value() && dvojceBlok->mtbIns[mtbInKontrolaMinus].value();
        }
    } else {
        if (simulace) {
            r[KP] = simulacePoloha == 1;
            r[KM] = simulacePoloha == 2;
        } else {
            r[KP] = mtbIns[mtbInKontrolaPlus].value();
            r[KM] = mtbIns[mtbInKontrolaMinus].value();
        }
    }
    r[RP] = mtbIns[mtbInRadicPlus].value();
    r[RM] = mtbIns[mtbInRadicMinus].value();
    r[RN] = mtbIns[mtbInRadicNouzove].value();

    // závěry z jiných bloků
    r[Z] = false;
    r[Z] |= r[ZP];
    if (!r[ZP])
    for (Tblok* &b : odvratneBloky) {
        if (b->typ == btS) {
            r[Z] |= b->r[TblokS::rel::Z];
            if (!b->r[TblokS::rel::Z]) odvratneBloky.removeOne(b);
        }
    }
    if (rezimSlave) {
        r[Z] |= dvojceBlok->r[ZP];
    }
    if (rezimMaster) {
        r[Z] |= dvojceBlok->r[ZP];
    }

    // pokud je stavění, nemůže být protilehlá kontrola
    r[KP] &= !r[SM];
    r[KM] &= !r[SP];

    // stavění výhýbky
    r[SP] &= !(r[SM] || r[KP]);
    if (rezimSlave) {
        r[SP] |= (dvojceBlok->r[KP] && !dvojceBlok->r[SM]);
    }
    r[SP] |= (r[RP] && !r[J]) || (r[RP] && r[J] && r[RN]) || (!r[RP] && !r[RM] && r[VOP] && !r[J]);
    r[SP] &= !(r[KP] || r[Z] || r[BP]);

    r[SM] &= !(r[SP] || r[KM]);
    if (rezimSlave) {
        r[SM] |= (dvojceBlok->r[KM] && !dvojceBlok->r[SP]);
    }
    r[SM] |= (!r[RP] && r[RM] && !r[J]) || (!r[RP] && r[RM] && r[J] && r[RN]) || (!r[RP] && !r[RM] && !r[VOP] && r[VOM] && !r[J]);
    r[SM] &= !(r[KM] || r[Z] || r[BP]);

    // dohledové relé
    if (rezimMaster) {
            r[DP] = r[KP] && !r[KM] && !r[SP] && !r[SM] &&  dvojceBlok->r[KP] && !dvojceBlok->r[KM] && !dvojceBlok->r[SP] && !dvojceBlok->r[SM];
            r[DM] = !r[KP] && r[KM] && !r[SP] && !r[SM] && !dvojceBlok->r[KP] &&  dvojceBlok->r[KM] && !dvojceBlok->r[SP] && !dvojceBlok->r[SM];
    } else {
        if (rezimSlave) {
            r[DP] = r[KP] && !r[KM] && !r[SP] && !r[SM] &&  dvojceBlok->r[KP] && !dvojceBlok->r[KM] && !dvojceBlok->r[SP] && !dvojceBlok->r[SM];
            r[DM] = !r[KP] && r[KM] && !r[SP] && !r[SM] && !dvojceBlok->r[KP] &&  dvojceBlok->r[KM] && !dvojceBlok->r[SP] && !dvojceBlok->r[SM];
        } else {
            // samostatná výhybka
            r[DP] = r[KP] && !r[KM] && !r[SP] && !r[SM];
            r[DM] = !r[KP] && r[KM] && !r[SP] && !r[SM];
        }
    }

    // indikace na panelu
    if (rezimSlave) {
        r[IP] = ((rKPV || dvojceBlok->r[RP]) && (r[KP] && !r[KM]));
        r[IM] = ((rKPV || dvojceBlok->r[RM]) && (r[KM]));
    } else {
        r[IP] = ((rKPV || r[RP]) && (r[KP] && !r[KM] && !r[SM]));
        r[IM] = ((rKPV || r[RM]) && (r[KM] && !r[KP] && !r[SP]));
    }
    r[INe] = !(r[DP] || r[DM]);

    // vstup průchozích průsvitek, z předchozího bloku a J, V
    if (predBlok) {
        if (predBlok->typ == btV) {
            if (predBlokMinus) {
                r[PB] = predBlok->r[TblokV::rel::PBM];
                r[PC] = predBlok->r[TblokV::rel::PCM];
            } else {
                r[PB] = predBlok->r[TblokV::rel::PBP];
                r[PC] = predBlok->r[TblokV::rel::PCP];
            }
            r[J] = predBlok->r[TblokV::rel::J];
            r[ZP] = predBlok->r[TblokV::rel::Z];

        }
        if (predBlok->typ == btS) {
            r[PB] = predBlok->r[TblokS::rel::PrB];
            r[PC] = predBlok->r[TblokS::rel::PrC];
            r[J] = predBlok->r[TblokS::rel::J];
            r[ZP] = predBlok->r[TblokS::rel::Z];
        }
    } else {
        r[ZP] = false;
    }

    // průsvitky průchozí
    r[PBP] = r[PB] && r[DP];
    r[PBM] = r[PB] && r[DM];
    r[PCP] = r[PC] && !r[DM];
    r[PCM] = r[PC] && !r[DP];

    // průsvitky místní - poloha
    if (rezimMaster) {
        r[PrCP] = (r[KP] && dvojceBlok->r[KP]) ? r[PC] : ((!r[KM] || !dvojceBlok->r[KM]) && rBlik50) ;
        r[PrCM] = (r[KM] && dvojceBlok->r[KM]) ? r[PC] : ((!r[KP] || !dvojceBlok->r[KP]) && rBlik50) ;
        /*
        if (r[KP] && dvojceBlok->r[KP]) {
            r[PrCP] =  r[PC];
        } else {
            r[PrCP] = ((!r[KM] || !dvojceBlok->r[KM]) && rBlik50);
            bBlikUsed |= (!r[KM] || !dvojceBlok->r[KM]);
        }
        if (r[KM] && dvojceBlok->r[KM]) {
            r[PrCM] = r[PC];
        } else {
            r[PrCM] = ((!r[KP] || !dvojceBlok->r[KP]) && rBlik50);
            bBlikUsed |= (!r[KP] || !dvojceBlok->r[KP]);
        }
        */

    } else {
        r[PrCP] = (r[KP]) ? r[PC] : (!r[KM] && rBlik50) ;
        r[PrCM] = (r[KM]) ? r[PC] : (!r[KP] && rBlik50) ;
        //bBlikUsed |= (!r[KP] && !r[KM]);
    }

    // simulace stavění
    if (simulace) {
        if ((simulacePoloha != 2) && r[SM]) {
            simulacePoloha = 0;
            if (!tim->isActive()) tim->start();
        }
        if ((simulacePoloha != 1) && r[SP]) {
            simulacePoloha = 0;
            if (!tim->isActive()) tim->start();
        }
    }

    // simulace odběru přestavníku
    if (r[SP] || r[SM]) {
        prestavnyProud = config.tProudPrestavniku;
    } else {
        prestavnyProud = 0;
    }

    // výstupy do MTB
    mtbOut[mtbOutStaveniPlus].setValueBool(r[SP]);
    mtbOut[mtbOutStaveniMinus].setValueBool(r[SM]);
    mtbOut[mtbOutIndikacePlus].setValueBool(r[IP]);
    mtbOut[mtbOutIndikaceMinus].setValueBool(r[IM]);
    mtbOut[mtbOutIndikaceNepoloha].setValueBool(r[INe]);
    mtbOut[mtbOutPrusvPlusCervena].setValueBool(r[PrCP]);
    mtbOut[mtbOutPrusvMinusCervena].setValueBool(r[PrCM]);

    if (r != rLast) return true; else return false;
}

void TblokV::onTimTimeout()
{
    if (r[SM]) simulacePoloha=2;
    if (r[SP]) simulacePoloha=1;
    if (!r[SP] && !r[SM]) simulacePoloha = 1; // po startu do +
}
