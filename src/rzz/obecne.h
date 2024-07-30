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
        if ((addr < 0) || (pin < 0)) {
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
        return mtb.module[addr].inputs[pin];
    }
    uint8_t valueOut() {
        if (!valid) return false;
        return mtb.module[addr].outputs[pin];
    }
    bool valueOutBool() {
        if (!valid) return false;
        return (mtb.module[addr].outputs[pin] > 0);
    }
    void setValue(uint8_t val) {
        if (!valid) return;
        if (mtb.module[addr].outputs[pin] != val) {
            mtb.module[addr].outputs[pin] = val;
            mtb.setOutput(addr, pin, val);
        }
    }
    void setValueBool(bool valbool) {
        if (!valid) return;
        uint8_t val = (valbool) ? 1 : 0;
        if (mtb.module[addr].outputs[pin] != val) {
            mtb.module[addr].outputs[pin] = val;
            mtb.setOutput(addr, pin, val);
        }
    }

};

//typedef struct Tmtbpin mtbpin;

extern bool rKPV; // konrola poloy výměn
extern bool rRV; // rušení volby
extern bool rZ3V; // NUZ, je něco vybráno
extern bool rQTV; // NUZ, probíhá měření času
extern bool rD3V; // NUZ, odměřeno, zruší se závěry
extern bool rBlik50; // výstup kmitače

class Tblok;
extern QList<Tblok *> bl;

#endif // OBECNE_H
