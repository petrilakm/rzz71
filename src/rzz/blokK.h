#ifndef BLOKS_K
#define BLOKS_K

#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokK : public Tblok
{
public:
    TblokK();

    enum mtbeIns {
        mtbInObsaz = 0,
    };
    enum mtbeOut {
        mtbOutBila = 0,
        mtbOutCervenaStred = 1,
        mtbOutCervenaKraje = 2,
    };

    bool evaluate() override;

    enum rel {A, B, V, R, J, U, X1, X2};
    #define RELAY_COUNT_K (8)
};

#endif // BLOKS_K
