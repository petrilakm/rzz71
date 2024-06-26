#include "blok.h"

//Tvoliciskupina *pvoliciskupina;

QList<Tblok *> bl;

Tblok::Tblok() {
    for(int i = 0; i < BLOK_MTB_MAX; i++) {
        mtbIns[i].valid = false;
    }
    name = "nil";
}

bool Tblok::evaluate()
{
    return false;
}

Tblok* Tblok::findBlokByName(QString name)
{
    foreach (Tblok *b, bl) {
        if (b->name == name) return b;
    }
    return NULL;
}
