#-------------------------------------------------
#
# Project created by QtCreator 2013-11-26T14:55:03
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = test_console
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
SOURCES += test_console.cpp \
    automata.cpp \
    poolalloc.cpp \
    transmissioncollection.cpp \
    incomingtransmitions.cpp \
    automata_minimize.cpp \
    automata_to_dfa.cpp \
    automata_to_c_code.cpp \
    automata_operations.cpp \
    automata_find.cpp \
    reparser.cpp \
    automata_serialization.cpp
HEADERS += \
    transition.h \
    state.h \
    automata.h \
    common.h \
    poolalloc.h \
    transmissioncollection.h \
    incomingtransmitions.h \
    find_lib.h \
    automata_types.h \
    reparser.h
win32:{
    LIBS += C:/boost_1_55_0/bin.v2/libs/regex/build/gcc-mingw-4.8.0/release/link-static/threading-multi/libboost_regex-mgw48-mt-1_55.a
    INCLUDEPATH += C:/boost_1_55_0/
    DEFINES += WINDOWS
}
unix:{
    DEFINES +=UNIX
    LIBS += -lboost_regex
    LIBS += /home/stiv/ttt/untitled/gen_find_lib/libfind_lib.a
}
