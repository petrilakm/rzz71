#ifndef BLOKRC_H
#define BLOKRC_H

#include "rzz/blok.h"

class TblokRC : public Tblok
{
    Q_OBJECT
public:
    explicit TblokRC();

    enum mtbeIns {
        mtbInRC = 0,
    };
    enum mtbeOut {
        mtbOutRC = 0,
    };

    bool evaluate() override;

    enum rel {EV};
    #define RELAY_COUNT_RC (1)

    QList<int> cestyRC;

};

#endif // BLOKRC_H
