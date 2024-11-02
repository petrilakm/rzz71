#include "lcd.h"
#include "logging.h"
#include "rzz/obecne.h"
#include "rzz71.h"
#include "rzz/dohledcesty.h"
#include <QFile>

Tlcd::Tlcd(QObject *parent)
    : QObject{parent}
{
    message_critical = "crit";
    message_warning = "warn";
    message_info = "info";

    tim = new QTimer(this);
    tim->setInterval(200);
    tim->setSingleShot(false);
    connect(tim, SIGNAL(timeout()), this, SLOT(on_tim()));
    tim->start();

#ifndef Q_OS_WIN
    if (QFile::exists(lcdname)) {
        log("lcd: found usable lcd, try it", logging::LogLevel::Info);
        fil.setFileName(lcdname);
        use_lcd = fil.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate | QIODeviceBase::ExistingOnly | QIODeviceBase::Unbuffered);
        if (use_lcd) {
            log("lcd: lcd opened", logging::LogLevel::Info);
            QByteArray tmp;
            tmp.append(lcdEscapeSeq);
            tmp.append('D'); // display ON
            tmp.append(lcdEscapeSeq);
            tmp.append('c'); // cursor OFF
            tmp.append(lcdEscapeSeq);
            tmp.append('+'); // backlight ON;
            tmp.append("\f"); // clear all
            fil.write(tmp); // send settings
            fil.write("init");
        } else {
            log(tr("lcd: can\'t open lcd device, %1").arg(fil.errorString()), logging::LogLevel::Error);
        }
    }
#endif
    // fill buffer with spaces (empty display)
    for(int i=0; i<(20); i++) {
        for(int j=0; j<(4); j++) {
            lcd_buffer[j].append('x');
        }
    }

}

Tlcd::~Tlcd()
{
  fil.close();
}

void Tlcd::on_tim()
{
    QByteArray old_buf[20];
    for(int i=0; i<20; i++) {
        old_buf[i] = lcd_buffer[i];
    }
    int linecur = 1;
    QString line;
    for (TdohledCesty::cestaPodDohledem *cpd : dohledCesty.cestyPostavene) {
        QString cestaName = QString("%1_%2").arg(cpd->pCesta->tlacitka[0]->name).arg(cpd->pCesta->tlacitka[1]->name);
        line = QString("%1 > %2-%3").arg(cestaName).arg(cpd->stav).arg(dohledCesty.stavCesty2QString(cpd->stav));
        lcd_buffer[linecur] = line.left(20).toLatin1();
        linecur++;
        for (QString upo1 : cpd->upo) {
            line = tr("UPO %1").arg(upo1);
            lcd_buffer[linecur] = line.left(20).toLatin1();
            linecur++;
        }
    }
    // clear remaining lines, if any
    for (int i = linecur; i<4; i++) {
        lcd_buffer[i] = " ";
    }
    if (rzz != nullptr) {
        // zobrazení časových souborů
        lcd_buffer[0] = "";
        int rem = rzz->t3V.remainingTime();
        if (rem != -1) {
            lcd_buffer[0] = QString("T3V = ").toLatin1() + QString::number(rem / 1000).toLatin1() + QString(" ").toLatin1();
        }
        rem = rzz->t3C.remainingTime();
        if (rem != -1) {
            lcd_buffer[0] = QString("T3C = ").toLatin1() + QString::number(rem / 1000).toLatin1() + QString(" ").toLatin1();
        }
        rem = rzz->t1C.remainingTime();
        if (rem != -1) {
            lcd_buffer[0] = QString("T1C = ").toLatin1() + QString::number(rem / 1000).toLatin1() + QString(" ").toLatin1();
        }
        rem = rzz->t5C.remainingTime();
        if (rem != -1) {
            lcd_buffer[0] = QString("T5C = ").toLatin1() + QString::number(rem / 1000).toLatin1() + QString(" ").toLatin1();
        }

        // zkrat (má přednost, proto na konec
        if (rZkrat) lcd_buffer[0] = tr("!! ZKRAT !! ZKRAT !!").left(20).toLatin1();
    } else {
        lcd_buffer[0] = " --- ";
    }



    bool same=true;
    for(int i=0; i<20; i++) {
        if (old_buf[i] != lcd_buffer[i]) same = false;
    }

    if (!same) redraw();
}

void Tlcd::redraw()
{
    log("lcd: redraw", logging::LogLevel::Info);
    for(int i=0; i<4; i++) {
        QString line = QString("lcd: *%1").arg(lcd_buffer[i]);
        line.append(QString(" ").repeated(26-line.length()));
        line.append(QString("*"));
        log(line, logging::LogLevel::Info);
    }
#ifndef Q_OS_WIN
    if (use_lcd) {
        QByteArray tmp;
        tmp.append(lcdEscapeSeqBase);
        tmp.append('H'); // go to home
        fil.write(tmp); // send pos 0,0
        tmp.clear();
        tmp.append(lcdEscapeSeq);
        tmp.append('k');
        tmp.append(10); // prepare kill line and newline
        for(int i=0; i<4; i++) {
            fil.write(lcd_buffer[i]);
            fil.write(tmp);
        }
    }
#endif
}
