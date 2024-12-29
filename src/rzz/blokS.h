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
        mtbInObsaz = 0, // kolejový obvod
        mtbInNuz = 1, // tlačítko pro označení úseku pro NUZ
    };
    enum mtbeOut {
    };

    bool evaluate() override;
    void zrusZaver(); // aktivace časovače pro odpadení relé Z

    enum rel {Z, V, R, J, U, P, PrB, PrC};
    #define RELAY_COUNT_S (8)
    // Z - závěr úseku
    // V - vybráno pro NUZ
    // R - rušící (asi nepouzito)
    // J - obsazeno
    // U - ??? (asi nepoužito)
    // P - projetá cesta
    // PrB - stav průsvitek bílých
    // PrC - stav průsvitek červených

    bool typM; // true = blok M, false = blok S

private:
    QTimer *zpozdeniReleZ; // zpožděný odpad relé Z

private slots:
    void slotReleZpritah();
};

#endif // BLOKS_S
