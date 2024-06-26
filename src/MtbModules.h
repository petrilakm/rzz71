#ifndef MTBMODULES_H
#define MTBMODULES_H

#include <QStringList>

class TMtbModuleConfigGeneric {
public:
    //int mtbType = -1;
};

struct sServoPosition {
    int posA;
    int posB;
};

struct sOutputConf {
    int type;
    int value;
};

class TMtbModuleConfigUNI : public TMtbModuleConfigGeneric {
public:
    TMtbModuleConfigUNI();
    int inputsDelay[16];
    int inputsType[16];
    struct sOutputConf outputsSafe[16];
};

class TMtbModuleConfigUNIS : public TMtbModuleConfigGeneric {
public:
    TMtbModuleConfigUNIS();
    int inputsDelay[16];
    struct sOutputConf outputsSafe[16+6*2];
    int servoEnabledMask;
    struct sServoPosition servoPosition[6];
    int servoSpeed[6];
};

class TMtbModuleState {
    public:
    /*
    enum mtbOutputType {
        mtbOutputPlain,
        mtbOutputScom
    };*/
    int address;
    int type;
    QString name;
    bool active;
    bool warning;
    bool error;
    TMtbModuleConfigGeneric *config;
};

class TMtbModuleTypes {
public:
    enum mtbModuleType {
        mtbTypeUNIv2 = 0x10,
        mtbTypeUNIv2noIR = 0x11,
        mtbTypeUNIv4 = 0x15,
        mtbTypeUNIv42 = 0x16,
        mtbTypeBOOST = 0x20,
        mtbTypeRC = 0x30,
        mtbTypeLC = 0x40, // level crossing
        mtbTypeUNIS = 0x50
    };
    static const int map[8];
    static const int count = 8;

    static const QStringList names;

    // output types
    enum mtbModuleUniOutputType {
        mtbOutputTypePlain = 0,
        mtbOutputTypeScom = 1,
    };
    static const int outputTypeMap[2];
    static const int OutputTypeCount;
    static const QStringList OutputTypeNames;

    // input types
    enum mtbModuleUniInputType {
        mtbInputTypePlain = 0,
        mtbInputTypeIR = 1
    };
    static const int InputTypeMap[2];
    static const int InputTypeCount;
    static const QStringList InputTypeNames;

    // conversion functions
    static int IndexToType(int index);
    static int TypeToIndex(int type);
    static QString GetNameByIndex(int index);
    static QString GetNameByType(int type);

    static QString ModuleUniOutputTypeGetName(int type);
    static QString ModuleUniOutputTypeGetNameFromIndex(int index);
    static QString ModuleUniInputTypeGetName(int type);
    static QString ModuleUniInputTypeGetNameFromIndex(int index);
    static int ModuleUniOutputTypeGetType(QString typeName);
    static int ModuleUniInputTypeGetType(QString typeName);
    static int ModuleUniOutputTypeToIndex(int type);
    static int ModuleUniOutputIndexToType(int index);
    static int ModuleUniInputTypeToIndex(int type);
    static int ModuleUniInputIndexToType(int index);
};

#endif // MTBMODULES_H
