#ifndef LCD_H
#define LCD_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include "rzz71.h"

class Tlcd : public QObject
{
    Q_OBJECT
public:
    explicit Tlcd(QObject *parent = nullptr);
    ~Tlcd();
    bool use_lcd = false;
    TRZZ71 *rzz = nullptr;
    QString message_critical;
    QString message_warning;
    QString message_info;
    void redraw();
    QByteArray lcd_buffer[40];
private:
    QFile fil;
    QTimer *tim;
    const QString lcdname = "/dev/lcd";
    const QByteArray lcdEscapeChar = QByteArray("\x1b",1);
    const QByteArray lcdEscapeSeq = QByteArray("\x1b[L",3);
    const QByteArray lcdEscapeSeqBase = QByteArray("\x1b[",2);
private slots:
    void on_tim();
};

#endif // LCD_H
