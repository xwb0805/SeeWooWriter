QT       += core gui widgets uitools

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = MouseWriterCpp
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp mainwindow.cpp pixeldrawer.cpp mousehook.cpp keyboardhook.cpp
HEADERS += mainwindow.h pixeldrawer.h mousehook.h keyboardhook.h

FORMS += mainwindow.ui

RESOURCES +=

LIBS += -lgdi32