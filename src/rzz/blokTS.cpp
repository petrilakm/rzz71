#include "blokTS.h"

TblokTS::TblokTS()
{
    typ = btTS;
    for (int i = 0; i < RELAY_COUNT_TS; ++i) {
        r.append(false);
    }
}

bool TblokTS::evaluate()
{
    return false;
}
