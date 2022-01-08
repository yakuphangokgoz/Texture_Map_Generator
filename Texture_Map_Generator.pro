QT       += core gui opengl charts


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    grayscale.cpp \
    histogram.cpp \
    main.cpp \
    mainwindow.cpp \
    normalmap.cpp

HEADERS += \
    blurvalues.h \
    grayscale.h \
    histogram.h \
    mainwindow.h \
    normalmap.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    Texture_Map_Generator_tr_TR.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

LIBS += -Lqtopencl/lib -Lqtopencl/lib/x86_64 -Lqtopencl/bin -lQtOpenCL -lOpenCL

INCLUDEPATH += qtopencl/src/opencl qtopencl/src/openclgl
DEPENDPATH += qtopencl/src/opencl qtopencl/src/openclgl

RC_ICONS = texroyal.ico

DISTFILES +=
