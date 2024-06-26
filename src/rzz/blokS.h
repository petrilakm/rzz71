#ifndef BLOKS_H
#define BLOKS_H

//#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokS : public Tblok
{
public:
    TblokS();

    enum mtbeIns {
        mtbInObsaz = 0,
    };
    enum mtbeOut {
      //  mtbOutBila = 0,
      //  mtbOutCervena = 1,
    };

    bool evaluate() override;

    enum rel {A, B, Z, V, R, J, U, PrB, PrC};
    #define RELAY_COUNT_S (9)

    bool typM;
};

#endif // BLOKS_S
