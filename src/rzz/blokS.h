#ifndef BLOKS_H
#define BLOKS_H

//#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokS : public Tblok
{
    Q_OBJECT
public:
    TblokS();

    enum mtbeIns {
        mtbInObsaz = 0,
        mtbInNuz = 1,
    };
    enum mtbeOut {
    };

    bool evaluate() override;
    void zrusZaver();

    enum rel {Z, V, R, J, U, PrB, PrC};
    #define RELAY_COUNT_S (7)

    bool typM;

private:
    QTimer *zpozdeniReleZ;

private slots:
    void slotReleZpritah();
};

#endif // BLOKS_S
