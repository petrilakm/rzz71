#include "cesty.h"

Tcesty::Tcesty(){
    load();
}

void Tcesty::load()
{
    Tcesta *pC;
    TblokTC *pTC;
    TblokV *pV;
    TblokS *pS;
    Tblok *pB;
    int cnt = 0;
    QStringList linelist;
    QStringList lineTC;
    QStringList lineV;
    QStringList lineO;
    QStringList lineB;
    QStringList lineNav;
    QString lineNavest;
    QFile inputFile("cesty.csv");
    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&inputFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.startsWith("#")) {
                continue;
            }
            linelist = line.split(";");
            if (linelist.count() < 7) {
                continue;
            }
            lineTC = linelist[1].split(','); // tlačítka
            lineV  = linelist[2].split(','); // výhybky
            lineO  = linelist[3].split(','); // odvraty
            lineB  = linelist[4].split(','); // bloky
            lineNav  = linelist[5].split(','); // návěstidla
            lineNavest = linelist[6]; // návěst

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
            // 2 - polohy výhybek
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
            // 3 - odvraty a jejich bloky
            for(int i=0; i < lineO.count(); i+=2) {
            //foreach (QString sO, lineO) {
                QString sBlokS;
                QString sBlokV;
                QString poloha;
                if ((i+1) >= lineO.count()) continue;
                sBlokV = lineO.at(i);
                sBlokS = lineO.at(i+1);
                if (sBlokV.endsWith('+') || sBlokV.endsWith('-')) {
                    poloha = sBlokV.last(1);
                    sBlokV.chop(1); // cut last character
                } else {
                    poloha = "+";
                    log(QString("cesty: cesta %1 nemá definovanou polohu odvratné výhybky \"%2\"").arg(pC->num).arg(sBlokV), logging::LogLevel::Warning);
                }

                pB = static_cast<Tblok*>(Tblok::findBlokByName(sBlokV));
                if (!pB) {
                    log(QString("cesty: nelze najít blokV \"%1\" pro odvrat cesty %2").arg(sBlokV).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pS = static_cast<TblokS*>(Tblok::findBlokByName(sBlokS));
                if (!pS) {
                    log(QString("cesty: nelze najít blokS/M \"%1\" pro odvrat cesty %2").arg(sBlokS).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                Tcesta::Tvyh_odv v;
                v.minus = (poloha == "-");
                v.pBlok = pB;
                v.pBlokS = pS;
                pC->odvraty.append(v);
            }

            // 4 - bloky cesty
            foreach (QString sB, lineB) {
                pB = Tblok::findBlokByName(sB);
                if (!pB) {
                    log(QString("cesty: nelze najít blok \"%1\" pro cestu %2").arg(sB).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pC->bloky.append(pB);
            }

            // 5 - návestidla (co stavíme, případně na kterém jsme závislí)
            pC->Navestidlo = nullptr;
            pC->nasledneNavestidlo = nullptr;
            if (lineNav.count() > 0) {
                pC->Navestidlo = static_cast<TblokQ*>(Tblok::findBlokByName(lineNav[0]));
            }
            if (lineNav.count() > 1) {
                pC->Navestidlo = static_cast<TblokQ*>(Tblok::findBlokByName(lineNav[1]));
            }

            // 6 - návěstní znak povolující jízdu
            bool ok;
            pC->navZnak = lineNavest.toInt(&ok);
            if (!ok) pC->navZnak = 5;


            cesty.append(pC);

        }
    }

}

Tcesty *cesty;
