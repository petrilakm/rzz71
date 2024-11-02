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
    tim->setInterval(300);
    tim->setSingleShot(false);
    connect(tim, SIGNAL(timeout()), this, SLOT(on_tim()));
    tim->start();

#ifndef Q_OS_WIN
    if (QFile::exists(lcdname)) {
        log("lcd: found usable lcd, try it", logging::LogLevel::Info);
        fil.setFileName(lcdname);
        use_lcd = fil.open(QIODeviceBase::WriteOnly || QIODeviceBase::Truncate || QIODeviceBase::ExistingOnly || QIODeviceBase::Unbuffered);
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
        }
    }
    for(int i=0; i<(20); i++) {
        for(int j=0; j<(4); j++) {
            lcd_buffer[j].append(' ');
        }
    }
#endif
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
        line = QString(" - %1 -> stav %2-%3").arg(cpd->num).arg(cpd->stav).arg(dohledCesty.stavCesty2QString(cpd->stav));
        lcd_buffer[linecur] = line.left(20).toLatin1();
        linecur++;
        for (QString upo1 : cpd->upo) {
            line = tr("UPO-%1").arg(upo1);
            lcd_buffer[linecur] = line.left(20).toLatin1();
            linecur++;
        }
    }
    if (rzz != nullptr) {
        int rem = rzz->t3V.remainingTime();
        if (rem == -1) {
            lcd_buffer[0] = " ";
        } else {
            lcd_buffer[0] = QString("T3V = ").toLatin1() + QString::number(rem / 1000).toLatin1();
        }
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

#ifndef Q_OS_WIN
    if (use_lcd) {
        QByteArray tmp;
        tmp.append(lcdEscapeSeq);
        tmp.append("x0y0;"); // clear all
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
