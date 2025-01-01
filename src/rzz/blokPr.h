#ifndef BLOKPR_H
#define BLOKPR_H

#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokPr : public Tblok
{
    Q_OBJECT
public:
    explicit TblokPr(QObject *parent = nullptr);

    enum mtbeOut {
        mtbOutPrusvBila = 0,
        mtbOutPrusvCervena = 1,
    };

    bool evaluate() override;

    enum rel {PrB, PrC};
    #define RELAY_COUNT_Pr (2)
    // PrB - průsvitka bílá
    // PrC - průsvitka červená

    Tblok *predBlok;
    bool predBlokMinus; // false == plus

};

#endif // BLOKPR_H
