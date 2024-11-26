#ifndef BLOKTS_H
#define BLOKTS_H

#include "rzz/blok.h"

class TblokTS : public Tblok
{
    Q_OBJECT
public:
    //explicit TblokTS(QObject *parent = nullptr);
    explicit TblokTS();

    enum mtbeIns {
        mtbInObsaz = 0,
        mtbInZadost = 1,
    };
    enum mtbeOut {
        mtbOutScom = 0,
        mtbOutMakHorniZluta = 1,
        mtbOutMakZelena = 2,
        mtbOutMakCervena = 3,
        mtbOutMakBila = 4,
        mtbOutMakDNVC = 5,
    };

    bool evaluate() override;

    enum rel {VT,SA,SB};
    #define RELAY_COUNT_TS (3)
    // VT - volnost tratě
    // SA - souhlas A
    // SB - souhlas B
    // EV - evidence vjezdu do stanice

};

#endif // BLOKTS_H
