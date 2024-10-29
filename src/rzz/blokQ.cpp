#include "blokQ.h"

TblokQ::TblokQ() {
    typ = btQ;
    for (int i = 0; i < RELAY_COUNT_Q; ++i) {
        r.append(false);
    }
    navestniZnak = 0;
}

bool TblokQ::evaluate()
{
    QList<bool> rLast = r;
    // logika

    //r[J] = mtbIns[mtbInObsaz].value();

    //mtbOut[mtbOutCervena].setValueBool(false);
    //mtbOut[mtbOutBila].setValueBool(false);
    bool bHZ,bZ,bC,bB,bDN;
    bHZ=bZ=bC=bB=bDN=0;
    bBlikUsed=0;
    switch (navestniZnak) {
    case 0:  bC=1; break; // stuj
    case 1:  bZ=1; bDN=1; break; // volno
    case 2:  bHZ=1; bDN=1; break; // výstraha
    case 3:  bHZ=1; bDN=1; bBlikUsed=1; break; // očekávej 40
    case 4:  bZ=1; bDN=1; break; // 40 + volno
    case 6:  bHZ=1; bDN=1; break; // 40 + výstraha
    case 7:  bHZ=1; bDN=1; bBlikUsed=1; break; // 40 + očekávej 40
    case 8:  bC=1; bB=rBlik50; bBlikUsed=1; break; // přivolávačka
    case 9:  bB=1; break; // posun dovolen
    }
    mtbOut[mtbOutMakHorniZluta].setValueBool(bHZ);
    mtbOut[mtbOutMakZelena].setValueBool(bZ);
    mtbOut[mtbOutMakCervena].setValueBool(bC);
    mtbOut[mtbOutMakBila].setValueBool(bB);
    mtbOut[mtbOutMakDNVC].setValueBool(bDN);
    mtbOut[mtbOutScom].setValue(navestniZnak);

    if (r != rLast) return true; else return false;
}
