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
        mtbInNuz = 1,
    };
    enum mtbeOut {
    };

    bool evaluate() override;

    enum rel {Z, V, R, J, U, PrB, PrC};
    #define RELAY_COUNT_S (7)

    bool typM;
};

#endif // BLOKS_S
