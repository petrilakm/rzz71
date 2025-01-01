#ifndef BLOKTS_H
#define BLOKTS_H

#include "rzz/blok.h"

class TblokTS : public Tblok
{
    Q_OBJECT
public:
    explicit TblokTS(QObject *parent = nullptr);
    //explicit TblokTS();

    enum mtbeIns {
        mtbInObsaz = 0,
        mtbInSouhlasUdeleni = 1,
        mtbInSouhlasZruseni = 2,
        mtbInOdhlaska = 3,
        mtbInVzdaleneSouhlasUdeleni = 4,
        mtbInVzdaleneSouhlasZruseni = 5,
        mtbInVzdaleneOdhlaska = 6
    };
    enum mtbeOut {
        mtbOutUsekBila = 0,
        mtbOutUsekRuda = 1,
        mtbOutVolnost = 2,
        mtbOutSouhlasUdeleni = 3,
        mtbOutSouhlasPrijeti = 4,
        mtbOutOdhlaska = 5,
        mtbOutVzdaleneVolnost = 6,
        mtbOutVzdaleneSouhlasUdeleni = 7,
        mtbOutVzdaleneSouhlasPrijeti = 8,
        mtbOutVzdaleneOdhlaska = 9
    };

    bool evaluate() override;

    enum rel {VT,SA,SB,EV,EO,ImSP,ImSU,ImVT,ImOD,IvSP,IvSU,IvVT,IvOD};
    #define RELAY_COUNT_TS (13)
    // VT - volnost tratě
    // SA - souhlas A
    // SB - souhlas B
    // EV - evidence vjezdu do stanice
    // EO - evidence odjezdu
    // ImSP - indikace místní souhlas příjem
    // ImSU - indikace místní souhlas udělení
    // ImVT - indikace místní volnost tratě
    // ImOD - indikace místní odhláška
    // IvSP - indikace vzdálená souhlas příjem
    // IvSU - indikace vzdálená souhlas udělení
    // IvVT - indikace vzdálená volnost tratě
    // IvOD - indikace vzdálená odhláška

    QTimer *casovacObsluhy;
};

#endif // BLOKTS_H
