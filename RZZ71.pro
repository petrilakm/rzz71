TARGET = RZZ71
TEMPLATE = app
QT -= gui
QT = core network

CONFIG += c++17 console
CONFIG -= app_bundle
QMAKE_CXXFLAGS += -Wall -Wextra -pedantic -std=c++17
CXXFLAGS -= -fno-keep-inline-dllexport

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
  src/MtbModules.cpp \
  src/lcd.cpp \
  src/logging.cpp \
  src/main.cpp \
  src/qconsolelistener.cpp \
  src/rzz/blok.cpp \
  src/rzz/blokEMZ.cpp \
  src/rzz/blokK.cpp \
  src/rzz/blokKU.cpp \
  src/rzz/blokOs.cpp \
  src/rzz/blokPN.cpp \
  src/rzz/blokPr.cpp \
  src/rzz/blokRC.cpp \
  src/rzz/blokS.cpp \
  src/rzz/blokQ.cpp \
  src/rzz/blokTC.cpp \
  src/rzz/blokTS.cpp \
  src/rzz/blokV.cpp \
  src/rzz/cesty.cpp \
  src/rzz/dohledcesty.cpp \
  src/rzz/voliciskupina.cpp \
  src/rzz71.cpp \
  src/tcpconsole.cpp \
  src/tcpsocket.cpp \
  src/tmtbConnector.cpp

HEADERS += \
  src/MtbModules.h \
  src/QConsoleListener \
  src/lcd.h \
  src/logging.h \
  src/main.h \
  src/qconsolelistener.h \
  src/rzz/blok.h \
  src/rzz/blokEMZ.h \
  src/rzz/blokK.h \
  src/rzz/blokKU.h \
  src/rzz/blokOs.h \
  src/rzz/blokPN.h \
  src/rzz/blokPr.h \
  src/rzz/blokRC.h \
  src/rzz/blokS.h \
  src/rzz/blokQ.h \
  src/rzz/blokTC.h \
  src/rzz/blokTS.h \
  src/rzz/blokV.h \
  src/rzz/cesty.h \
  src/rzz/dohledcesty.h \
  src/rzz/obecne.h \
  src/rzz/voliciskupina.h \
  src/rzz71.h \
  src/tcpconsole.h \
  src/tcpsocket.h \
  src/termcolor.h \
  src/tmtbConnector.h

INCLUDEPATH += \
        src

#TRANSLATIONS += \
#    RZZ71_cs_CZ.ts
#CONFIG += lrelease
#CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

VERSION_MAJOR = 0
VERSION_MINOR = 1

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR" "VERSION_MINOR=$$VERSION_MINOR"

#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}
DEFINES += "VERSION=\\\"$${VERSION}\\\""
