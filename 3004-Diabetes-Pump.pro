QT += core gui charts widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = insulinpump
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    $$files(src/*.cpp, true)

HEADERS += \
    mainwindow.h \
    $$files(src/*.h, true)

FORMS += mainwindow.ui

CONFIG  += c++17
