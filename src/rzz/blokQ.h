#ifndef BLOKS_Q
#define BLOKS_Q

#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokQ : public Tblok
{
public:
    TblokQ();

    enum mtbeIns {
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

    enum rel {N};
    #define RELAY_COUNT_Q (1)

    int navestniZnak;
};

#endif // BLOKS_Q
