#-------------------------------------------------
#
# Project created by QtCreator 2015-07-18T19:20:16
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StereoCorrespondenceBMTuner
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

# by default, Qt does not support pkg-config for Mac OS
# so we must enable it
QT_CONFIG -= no-pkg-config

# we add the package opencv to pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += opencv
