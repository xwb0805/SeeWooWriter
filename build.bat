@echo off
set PATH=E:\qt\Tools\mingw1310_64\bin;E:\qt\6.10.0\mingw_64\bin;%PATH%
set QTDIR=E:\qt\6.10.0\mingw_64
qmake6 MouseWriterCpp.pro
mingw32-make -j%NUMBER_OF_PROCESSORS%