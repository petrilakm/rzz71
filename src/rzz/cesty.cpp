#include "cesty.h"

Tcesty::Tcesty(){
    load();
}

void Tcesty::load()
{
    Tcesta *pC;
    TblokTC *pTC;
    TblokV *pV;
    Tblok *pB;
    int cnt = 0;
    QStringList linelist;
    QStringList lineTC;
    QStringList lineV;
    QStringList lineB;
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
            if (linelist.count() < 4) {
                continue;
            }
            lineTC = linelist[1].split(',');
            lineV  = linelist[2].split(',');
            lineB  = linelist[3].split(',');

            pC = new Tcesta;
            pC->num = cnt++;
            foreach (QString sTC, lineTC) {
                pTC = static_cast<TblokTC*>(Tblok::findBlokByName(sTC));
                if (!pTC) {
                    log(QString("cesty: nelze najít tlačítko \"%1\" pro cestu %2").arg(sTC).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pC->tlacitka.append(pTC);
            }
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
                v.pBlokV = pV;
                pC->polohy.append(v);
            }
            foreach (QString sB, lineB) {
                pB = Tblok::findBlokByName(sB);
                if (!pB) {
                    log(QString("cesty: nelze najít blok \"%1\" pro cestu %2").arg(sB).arg(pC->num), logging::LogLevel::Error);
                    continue;
                }
                pC->bloky.append(pB);
            }
            cesty.append(pC);

        }
    }

}

Tcesty *cesty;
