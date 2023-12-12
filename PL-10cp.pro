TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        Funciones_t.c \
        main.c

SIMUL_LIBRARY_PATH = C:\Users\Abdelilah\Downloads\SimuladorSistemas-2023_PL7\Client\Library
INCLUDEPATH += $${SIMUL_LIBRARY_PATH}/include
contains(QT_ARCH, i386) {
    message("32-bit")
    LIBS += -L$${SIMUL_LIBRARY_PATH}/mingw32 -lUserLibSimulator
} else {
    message("64-bit")
    LIBS += -L$${SIMUL_LIBRARY_PATH}/mingw64 -lUserLibSimulator
}
# Si se desea utilizar curses:
LIBS += -lpdcurses
# Si se desea utilizar windows sockets:
LIBS += -lws2_32

HEADERS += \
    MisFunciones.h
