#include "blokKU.h"

// Blok přibližovacho úseku (kolejový usek)

TblokKU::TblokKU(QObject *parent)
    : Tblok{parent}
{
    typ = btKU;
    for (int i = 0; i < RELAY_COUNT_KU; ++i) {
        r.append(false);
    }

    timZvonek = new QTimer(this);
    timZvonek->setSingleShot(true);
    timZvonek->setInterval(config.tZvonek);
    connect(timZvonek, SIGNAL(timeout()), this, SLOT(slotZvonekKonec()));
}

bool TblokKU::evaluate()
{
    QList<bool> rLast = r;
    // vstupy
    r[J] = mtbIns[mtbInObsaz].value();

    // logika
    // obsazení zapíná zvonek, pokud není odjezd
    r[Zv] |= (r[J] && !r[OC] && !r[ZP]);
    // zvonek zapne časovač
    if (r[Zv] && !r[ZP]) {
        timZvonek->start();
        r[ZP] = true;
    }

    r[Zv] &= r[J]; // volný usek resetuje zvonek
    r[ZP] &= r[J]; // volný usek resetuje zvonek
    r[OC] &= (r[J] || (!r[EVO])); // OC se resetuje volným úsekem a proběhlým odjezdem
    r[OCP] |= r[OC] && (!r[J]); // pro správne vyhodnocení EVO, musí být úsek někdy volný
    r[OCP] &= r[OC]; // odpadem OC odpadne i OCP
    r[EVO] &= r[J]; // volný úsek resetuje EVO
    r[EVO] |= r[J] && r[OC] && r[OCP];

    // vystupy
    mtbOut[mtbOutIndBila].setValueBool(!r[J]);
    mtbOut[mtbOutIndCervena].setValueBool(r[J]);
    mtbOut[mtbOutZvonec].setValueBool(r[Zv]);
    if (r != rLast) return true; else return false;
}

void TblokKU::slotZvonekKonec()
{
    r[Zv] = false;
}
