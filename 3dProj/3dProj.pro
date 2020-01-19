QT += 3dcore 3drender 3dinput 3dextras
QT += widgets

SOURCES += main.cpp \
    scenemodifier.cpp \
    draw3dlines.cpp

HEADERS += \
    scenemodifier.h \
    draw3dlines.h

RESOURCES += \
    axes.qrc

RC_ICONS +=\
    icon.ico

DISTFILES += \
    icon.ico
