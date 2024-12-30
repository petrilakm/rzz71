#include "cesty.h"

bool Tcesta::zjistiObsazeni(int usek)
{
    bool obsaz = true;
    // useky mimo cestu jsou obsazené trvale
    if (usek < 0) return true;
    if (usek >= this->bloky.count()) return true;

    Tblok *blok = this->bloky[usek];
    if (blok->typ == Tblok::btS) {
        obsaz = static_cast<TblokS *>(blok)->r[TblokS::rel::J];
    }
    if (blok->typ == Tblok::btK) {
        obsaz = static_cast<TblokK *>(blok)->r[TblokK::rel::J];
    }
    return obsaz;
}

bool Tcesta::zjistiZaver(int usek)
{
    bool obsaz = true;
    // useky mimo cestu jsou bez závěru
    if (usek < 0) return false;
    if (usek >= this->bloky.count()) return false;

    Tblok *blok = this->bloky[usek];
    if (blok->typ == Tblok::btS) {
        obsaz = static_cast<TblokS *>(blok)->r[TblokS::rel::Z];
    }
    if (blok->typ == Tblok::btK) {
        obsaz = static_cast<TblokK *>(blok)->r[TblokK::rel::X1] || static_cast<TblokK *>(blok)->r[TblokK::rel::X2];
    }
    return obsaz;
}


void Tcesta::uvolniZaver(int usek)
{
    if (usek < 0) return;
    if (usek >= this->bloky.count()) return;

    Tblok *blok = this->bloky[usek];
    if (blok->typ == Tblok::btS) {
        static_cast<TblokS *>(blok)->zrusZaver(); // aktivace časovače
        //static_cast<TblokS *>(blok)->r[TblokS::rel::Z] = false;
    }
}

Tcesty::Tcesty(){
    load();
}

void Tcesty::load()
{
    Tcesta *pC;
    TblokTC *pTC;
    TblokV *pV;
    Tblok *pB;
    Tblok *pB2;
    int cnt = 0;
    QStringList linelist;
    QStringList lineTC;
    QStringList lineTCm;
    QStringList lineV;
    QStringList lineO;
    QStringList lineB;
    QStringList lineNav;
    QString lineNavest;
    // načte cesty
    QFile inputFile("cesty.csv");
    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine().trimmed();
            if (line.startsWith("#")) {
                continue;
            }
            linelist = line.split(";");
            if (linelist.count() < 8) {
                continue;
            }
            lineTC = linelist[1].trimmed().split(','); // tlačítka volící
            lineTCm.clear();
            linelist[2] = linelist[2].trimmed();
            if (linelist[2] != "") {
                lineTCm= linelist[2].split(','); // tlačítka mezilehlá, pro VA
            }
            lineV  = linelist[3].trimmed().split(','); // výhybky
            lineO  = linelist[4].trimmed().split(','); // odvraty
            lineB  = linelist[5].trimmed().split(','); // bloky
            lineNav  = linelist[6].trimmed().split(','); // návěstidla
            lineNavest = linelist[7].trimmed(); // návěst

            pC = new Tcesta;
            pC->num = cnt++;
            pC->posun = (linelist[0] != "V");
            // 1 - tlačítka cesty
            foreach (QString sTC, lineTC) {
                pTC = static_cast<TblokTC*>(Tblok::findBlokByName(sTC));
                if (!pTC) {
                    log(QString("cesty: nelze najít tlačítko \"%1\" pro cestu %2").arg(sTC).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pC->tlacitka.append(pTC);
            }
            // 2 - tlačítka mezilehlá pro VA
            foreach (QString sTC, lineTCm) {
                pTC = static_cast<TblokTC*>(Tblok::findBlokByName(sTC));
                if (!pTC) {
                    log(QString("cesty: nelze najít mezilehlé tlačítko \"%1\" pro cestu %2").arg(sTC).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pC->tlacitkaMezilehla.append(pTC);
            }
            // 3 - polohy výhybek
            foreach (QString sV, lineV) {
                QString sBlokV;
                QString poloha;
                if (sV.endsWith('+') || sV.endsWith('-')) {
                    poloha = sV.last(1);
                    sV.chop(1); // cut last character
                } else {
                    poloha = "+";
                    log(QString("cesty: cesta %1 nemá definovanou polohu výhybky \"%2\"").arg(pC->num).arg(sV), logging::LogLevel::Warning);
                }

                pV = static_cast<TblokV*>(Tblok::findBlokByName(sV));
                if (!pV) {
                    log(QString("cesty: nelze najít blokV \"%1\" pro cestu %2").arg(sV).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                Tcesta::Tvyh v;
                v.minus = (poloha == "-");
                v.pBlok = pV;
                pC->polohy.append(v);
            }
            // 4 - odvraty a jejich bloky
            for(int i=0; i < lineO.count(); i+=2) {
            //foreach (QString sO, lineO) {
                QString sBlokUsek;
                QString sBlokVymena;
                QString poloha;
                if ((i+1) >= lineO.count()) continue;
                sBlokVymena = lineO.at(i);
                sBlokUsek = lineO.at(i+1);
                if (sBlokVymena.endsWith('+') || sBlokVymena.endsWith('-')) {
                    poloha = sBlokVymena.last(1);
                    sBlokVymena.chop(1); // cut last character
                } else {
                    poloha = "+";
                    log(QString("cesty: cesta %1 nemá definovanou polohu odvratné výhybky \"%2\"").arg(pC->num).arg(sBlokVymena), logging::LogLevel::Warning);
                }

                pB = Tblok::findBlokByName(sBlokVymena);
                if (!pB) {
                    log(QString("cesty: nelze najít blokV/EMZ \"%1\" pro odvrat cesty %2").arg(sBlokVymena).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pB2 = Tblok::findBlokByName(sBlokUsek);
                if (!pB2) {
                    log(QString("cesty: nelze najít blokS/M/K \"%1\" pro odvrat cesty %2").arg(sBlokUsek).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                Tcesta::Tvyh_odv v;
                v.minus = (poloha == "-");
                v.pBlokVymena = pB;
                v.pBlokUsek = pB2;
                pC->odvraty.append(v);
            }

            // 5 - bloky cesty
            foreach (QString sB, lineB) {
                pB = Tblok::findBlokByName(sB);
                if (!pB) {
                    log(QString("cesty: nelze najít blok \"%1\" pro cestu %2").arg(sB).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pC->bloky.append(pB);
            }

            // 6 - návestidla (co stavíme, případně na kterém jsme závislí)
            pC->Navestidlo = nullptr;
            pC->nasledneNavestidlo = nullptr;
            if (lineNav.count() > 0) {
                pC->Navestidlo = static_cast<TblokQ*>(Tblok::findBlokByName(lineNav[0]));
                if (pC->Navestidlo == nullptr) log(QString("cesty: cesta %1 nemůže najít návěstidlo \"%2\"").arg(pC->num).arg(lineNav[0]), logging::LogLevel::Error);
            }
            if (lineNav.count() > 1) {
                pC->nasledneNavestidlo = static_cast<TblokQ*>(Tblok::findBlokByName(lineNav[1]));
                if (pC->nasledneNavestidlo == nullptr) log(QString("cesty: cesta %1 nemůže najít následující návěstidlo \"%2\"").arg(pC->num).arg(lineNav[1]), logging::LogLevel::Error);
            }

            // 7 - návěstní znak povolující jízdu
            bool ok;
            pC->navZnak = lineNavest.toInt(&ok);
            if (!ok) pC->navZnak = 5;

            cesty.append(pC);
        }
    }
}

Tcesty *cesty;
