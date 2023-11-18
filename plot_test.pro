QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    algorithm.cpp \
    colorlabel.cpp \
    job.cpp \
    jobgroup.cpp \
    main.cpp \
    mainwindow.cpp \
    plan.cpp \
    qcustomplot.cpp \
    worker.cpp \
    workergroup.cpp

HEADERS += \
    algorithm.h \
    colorlabel.h \
    job.h \
    jobgroup.h \
    mainwindow.h \
    plan.h \
    qcustomplot.h \
    worker.h \
    workergroup.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
