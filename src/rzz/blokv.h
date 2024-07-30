#ifndef BLOKV_H
#define BLOKV_H

#include "rzz/obecne.h"
#include "rzz/blok.h"
#include "rzz/blokS.h"

class TblokV : public Tblok
{
    Q_OBJECT
private:
    QTimer *tim;
public:
    TblokV();

    enum mtbeIns {
        mtbInKontrolaPlus = 0,
        mtbInKontrolaMinus = 1,
        mtbInRadicPlus = 2,
        mtbInRadicMinus = 3,
        mtbInRadicNouzove = 4,
    };
    enum mtbeOut {
        mtbOutStaveniPlus = 0,
        mtbOutStaveniMinus = 1,
        mtbOutIndikacePlus = 2,
        mtbOutIndikaceNepoloha = 3,
        mtbOutIndikaceMinus = 4,
        mtbOutPrusvPlusCervena = 5,
        mtbOutPrusvMinusCervena = 6,
    };

    bool evaluate() override;
    //        0   1   2   3   4   5   6  7  8    9    10  11  12  13  14  15  16  17  18   19   20   21   22    23
    enum rel {KP, KM, DP, DM, SP, SM, J, Z, VOP, VOM, RP, RM, RN, IP, IM, INe, PB, PC, PBP, PCP, PBM, PCM, PrCP, PrCM};
    #define RELAY_COUNT_V (24)
    // KP - kontrola +
    // KM - kontrola -
    // DP - dohled +
    // DM - dohled -
    // SP - stavění +
    // SM - stavění -
    // J - kolejové relé
    // Z - závěr
    // RP - řadič +
    // RM - řadič -
    // IP - indikace +
    // IM - indikace -
    // IN - indikace nepoloha
    // PB - vstup vodiče 7 (bílá)
    // PC - vstup vodiče 8 (červená)
    // PBP - výstup vodiče 7 plus (bílá)
    // PCP - výstup vodiče 8 plus (červená)
    // PrCP - průsvitka polohy červená plus
    // PrCM - průsvitka polohy červená mínus

    //TblokS *blokS; // pro obsazení a závěr
    Tblok *predBlok; // předchozí blok ve směru proti hrotu
    bool predZ;
    QList<Tblok *> odvratneBloky; // seznam bloků, kterým toto V tvoří odvrat
    bool predBlokMinus; // false == plus
    bool rezimMaster;
    bool rezimSlave;
    TblokV *dvojceBlok;

    bool simulace; // true - simulace výhybky
    int simulacePoloha; // 0 = neznamo, 1 = plus, 2 = minus
private slots:
    void onTimTimeout();
};

#endif // BLOKV_H
