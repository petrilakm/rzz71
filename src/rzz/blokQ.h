#ifndef BLOKS_Q
#define BLOKS_Q

#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokQ : public Tblok
{
public:
    TblokQ();

    enum mtbeIns {
        mtbInObsaz = 0,
    };
    enum mtbeOut {
        mtbOutHorniZluta = 0,
        mtbOutZelena = 1,
        mtbOutCervena = 2,
        mtbOutBila = 3,
        mtbOutDolniZluta = 4,
        mtbOutScom = 5,
    };

    bool evaluate() override;

    enum rel {A, B, V, R, J, U};
    #define RELAY_COUNT_Q (6)
};

#endif // BLOKS_Q
