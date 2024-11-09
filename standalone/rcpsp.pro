QT       += core
#QT       += charts
QT       += widgets
QT       += testlib
#QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 5):

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../algorithm
INCLUDEPATH += ../model
INCLUDEPATH += ../misc

SOURCES += \
    main.cpp \
    ../algorithm/*.cpp \
    ../model/*.cpp \
    ../misc/*.h

HEADERS += \
    ../algorithm/*.h \
    ../model/*.h \
    ../misc/*.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

Release:DESTDIR = release
Debug:DESTDIR = debug

OBJECTS_DIR = $${DESTDIR}
MOC_DIR = $${DESTDIR}
RCC_DIR = $${DESTDIR}
UI_DIR = $${DESTDIR}