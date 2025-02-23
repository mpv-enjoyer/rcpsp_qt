QT       += core gui
QT       += charts
QT       += widgets
QT       += testlib
QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 5):

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += algorithm
INCLUDEPATH += model
INCLUDEPATH += debugoutput

SOURCES += \
    algorithm/algorithm.cpp \
    algorithm/assignedjobs.cpp \
    algorithm/completedjobs.cpp \
    algorithm/pendingfronts.cpp \
    algorithm/pendingjobs.cpp \
    algorithm/loader.cpp \
    contentwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    misc/debugoutput.cpp \
    model/job.cpp \
    model/jobgroup.cpp \
    model/plan.cpp \
    model/worker.cpp \
    model/workergroup.cpp \
    plot.cpp \
    qcustomplot.cpp \
    chartview.cpp \

HEADERS += \
    algorithm/algorithm.h \
    algorithm/assignedjobs.h \
    algorithm/completedjobs.h \
    algorithm/pendingfronts.h \
    algorithm/pendingjobs.h \
    algorithm/loader.h \
    contentwidget.h \
    mainwindow.h \
    misc/debugoutput.h \
    model/job.h \
    model/jobgroup.h \
    model/plan.h \
    model/worker.h \
    model/workergroup.h \
    plot.h \
    qcustomplot.h \
    chartview.h \

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
