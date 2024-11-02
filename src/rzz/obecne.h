#ifndef OBECNE_H
#define OBECNE_H

#include <cstdint>
#include "logging.h"
#include "tmtbConnector.h"

class mtbpin {
public:
    bool valid = false;
    uint8_t addr;
    uint8_t pin;

    // creator
    mtbpin(int addr, int pin) {
        if ((addr < 0) || (pin < 0) || ((addr == 0) && (pin == 0))) {
            this->valid = false;
            this->addr = 0;
            this->pin = 0;
            return;
        }
        this->addr = addr;
        this->pin  = pin;
        this->valid = true;
    }
    // empty creator
    mtbpin() {
        this->addr = 0;
        this->pin  = 0;
        this->valid = false;
    }

    // operator ==
    bool operator==(const mtbpin a) const {
        if ((a.valid) && (valid) && (a.addr == addr) && (a.pin == pin)) return true;
        return false;
    }

    bool value() {
        if (!valid) return false;
        if (this->addr > 100) {
            //return mtb_stanice.module[addr-100].inputs[pin];
            return 0;
        }
        return mtb.module[addr].inputs[pin];
    }
    uint8_t valueOut() {
        if (!valid) return false;
        if (this->addr>100) {
            //return mtb_stanice.module[this->addr-100].outputs[pin];
            return 0;
        }
        return mtb.module[this->addr].outputs[pin];
    }
    bool valueOutBool() {
        if (!valid) return false;
        if (this->addr > 100) {
            //return (mtb_stanice.module[addr-100].outputs[pin] > 0);
            return 0;
        }
        return (mtb.module[this->addr].outputs[pin] > 0);
    }
    void setValue(uint8_t val) {
        if (!valid) return;
        if (this->addr > 100) {
            /*
            if (mtb_stanice.module[this->addr-100].outputs[pin] != val) {
                mtb_stanice.module[this->addr-100].outputs[pin] = val;
                mtb_stanice.setOutput(this->addr-100, pin, val);
            }
            */
        } else {
            if (mtb.module[addr].outputs[pin] != val) {
                mtb.module[addr].outputs[pin] = val;
                mtb.setOutput(addr, pin, val);
            }
        }
    }
    void setValueBool(bool valbool) {
        if (!valid) return;
        uint8_t val = (valbool) ? 1 : 0;
        if (this->addr > 100) {
            /*
            if (mtb_stanice.module[addr-100].outputs[pin] != val) {
                mtb_stanice.module[addr-100].outputs[pin] = val;
                mtb_stanice.setOutput(addr-100, pin, val);
            }
            */
        } else {
            if (mtb.module[addr].outputs[pin] != val) {
                mtb.module[addr].outputs[pin] = val;
                mtb.setOutput(addr, pin, val);
            }
        }
    }

};

//typedef struct Tmtbpin mtbpin;

extern bool rKPV; // kontrola polohy výměn
extern bool rZkrat; // zkrat na zesilovači (vypadek DCC)
extern bool rRV; // rušení volby
extern bool rZ3V; // NUZ, je něco vybráno
extern bool rQTV; // NUZ, probíhá měření času
extern bool rD3V; // NUZ, odměřeno, zruší se závěry
extern bool rBlik50; // výstup kmitače
extern bool rNavNoc; // noční návestidla

class Tblok;
extern QList<Tblok *> bl;

#endif // OBECNE_H
