#ifndef BLOKS_K
#define BLOKS_K

#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokK : public Tblok
{
public:
    explicit TblokK(QObject *parent = nullptr);

    enum mtbeIns {
        mtbInObsaz = 0,
        mtbInNuz = 1,
    };
    enum mtbeOut {
        mtbOutBila = 0,
        mtbOutCervenaStred = 1,
        mtbOutCervenaKraje = 2,
    };

    bool evaluate() override;

    enum rel {V, R, J, U, X1, X2, K1, K2};
    #define RELAY_COUNT_K (8)

    Tblok * predBlok1;
    Tblok * predBlok2;
};

#endif // BLOKS_K
