#include "blok.h"

//Tvoliciskupina *pvoliciskupina;

QList<Tblok *> bl;

Tblok::Tblok(QObject *parent) : QObject{parent} {
    for(int i = 0; i < BLOK_MTB_MAX; i++) {
        mtbIns[i].valid = false;
        mtbOut[i].valid = false;
    }
    typ = btNULL;
    name = "nil";
    bBlikUsed = false;
    for(int i = 0; i< r.count(); i++) {
        r[i] = false;
    }
}

bool Tblok::evaluate()
{
    return false;
}

Tblok* Tblok::findBlokByName(QString name)
{
    for(Tblok *b : bl) {
        if (b->name == name) return b;
    }
    return NULL;
}
