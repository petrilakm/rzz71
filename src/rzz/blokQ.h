#ifndef BLOKS_Q
#define BLOKS_Q

#include "rzz/obecne.h"
#include "rzz/blok.h"

class TblokQ : public Tblok
{
    Q_OBJECT
private:
    QTimer *tim;
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

    enum rel {N,Nreal,Fo};
    #define RELAY_COUNT_Q (3)
    // N - požadavek na návestní relé
    // Nreal - skutečné návestní relé (zpožděný odpad)
    // Fo - opakovač přivolávací návesti

    bool relLastFo;

    int navestniZnak; // co někdo chce, aby svítilo na návestidle
    int navestniZnakReal; // co bude skutečně svítit na návestidle
private slots:
    void onTimTimeout();
};

#endif // BLOKS_Q
