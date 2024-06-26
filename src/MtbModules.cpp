#include "MtbModules.h"

const QStringList TMtbModuleTypes::names = {
    "UNI v2",
    "UNI v2 no IR",
    "UNI v4.0",
    "UNI v4.2",
    "BOOST",
    "MTB-RC",
    "MTB-LC",
    "UNIS"
};
const int TMtbModuleTypes::map[8] = {0x10, 0x11, 0x15, 0x16, 0x20, 0x30, 0x40, 0x50};

// output types:
const int TMtbModuleTypes::outputTypeMap[2] = {0, 1};
const int TMtbModuleTypes::OutputTypeCount = 2;
const QStringList TMtbModuleTypes::OutputTypeNames = {
    "plain",
    "s-com"
};

// input types:
const int TMtbModuleTypes::InputTypeMap[2] = {0, 1};
const int TMtbModuleTypes::InputTypeCount = 2;
const QStringList TMtbModuleTypes::InputTypeNames = {
    "plain",
    "ir"
};

TMtbModuleConfigUNI::TMtbModuleConfigUNI()
{
    //this->mtbType = 0x10;
    for(int i = 0; i < 16; i++) {
        this->inputsDelay[i] = 0;
        this->inputsType[i] = 0;
        this->outputsSafe[i].type = 0;
        this->outputsSafe[i].value = 0;
    }
}

TMtbModuleConfigUNIS::TMtbModuleConfigUNIS()
{
    //this->mtbType = 0x50;
    for(int i = 0; i < 16; i++) {
        this->inputsDelay[i] = 0;
        this->outputsSafe[i].type = 0;
        this->outputsSafe[i].value = 0;
    }
    for(int i = 16; i < 28; i++) {
        this->outputsSafe[i].type = 0;
        this->outputsSafe[i].value = 0;
    }
}


int TMtbModuleTypes::IndexToType(int index)
{
    if((index >= 0) && (index < count)) {
        return map[index];
    }
    return -1;
}

int TMtbModuleTypes::TypeToIndex(int type)
{
    for(int i = 0; i < count; i++) {
        if (map[i] == type) return i;
    }
    return 0;
}

QString TMtbModuleTypes::GetNameByIndex(int index)
{
    if((index >= 0) && (index < count)) {
        return names[index];
    }
    return QString("-");
}

QString TMtbModuleTypes::GetNameByType(int type)
{
    int index = TypeToIndex(type);
    if (index > -1) {
        return GetNameByIndex(index);
    }
    return QString("-");
}

QString TMtbModuleTypes::ModuleUniOutputTypeGetName(int type)
{
    int index = ModuleUniOutputTypeToIndex(type);
    if (index > -1) {
        return OutputTypeNames.at(index);
    }
    return QString("bad_type");
}

QString TMtbModuleTypes::ModuleUniOutputTypeGetNameFromIndex(int index)
{
    if ((index > -1) && (index < OutputTypeCount)) {
        return OutputTypeNames.at(index);
    }
    return QString("bad_index");
}

QString TMtbModuleTypes::ModuleUniInputTypeGetName(int type)
{
    int index = ModuleUniInputTypeToIndex(type);
    if ((index > -1)) {
        return InputTypeNames.at(index);
    }
    return QString("bad_type");
}

QString TMtbModuleTypes::ModuleUniInputTypeGetNameFromIndex(int index)
{
    //int index = ModuleUniInput
    if ((index > -1)) {
        return InputTypeNames.at(index);
    }
    return QString("bad_index");
}


int TMtbModuleTypes::ModuleUniOutputTypeGetType(QString typeName)
{
    for(int i = 0; i < OutputTypeCount; i++) {
        if (OutputTypeNames.at(i) == typeName) return i;
    }
    return 0;
}

int TMtbModuleTypes::ModuleUniInputTypeGetType(QString typeName)
{
    for(int i = 0; i < InputTypeCount; i++) {
        if (InputTypeNames.at(i) == typeName) return i;
    }
    return 0;
}

int TMtbModuleTypes::ModuleUniOutputTypeToIndex(int type)
{
    for(int i = 0; i < OutputTypeCount; i++) {
        if (outputTypeMap[i] == type) {
            return i;
        }
    }
    return -1;
}

int TMtbModuleTypes::ModuleUniOutputIndexToType(int index)
{
    if (index < OutputTypeCount) {
        return outputTypeMap[index];
    } else {
        return -1;
    }
}

int TMtbModuleTypes::ModuleUniInputTypeToIndex(int type)
{
    for(int i = 0; i < InputTypeCount; i++) {
        if (InputTypeMap[i] == type) {
            return i;
        }
    }
    return -1;
}

int TMtbModuleTypes::ModuleUniInputIndexToType(int index)
{
    if (index < InputTypeCount) {
        return InputTypeMap[index];
    } else {
        return -1;
    }
}
