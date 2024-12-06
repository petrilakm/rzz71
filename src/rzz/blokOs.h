#ifndef BLOKOS_H
#define BLOKOS_H

#include "rzz/blok.h"

class TblokOs : public Tblok
{
    Q_OBJECT
public:
    explicit TblokOs();

    enum mtbeIns {
        mtbInOsv = 0,
    };
    enum mtbeOut {
        mtbOutInd = 0,
        mtbOutOsv = 1,
    };

    bool evaluate() override;

    enum rel {OSV};
    #define RELAY_COUNT_OS (1)
};

#endif // BLOKOS_H
