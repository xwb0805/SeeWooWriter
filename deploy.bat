@echo off
set PATH=E:\qt\Tools\mingw1310_64\bin;E:\qt\6.10.0\mingw_64\bin;%PATH%
set QTDIR=E:\qt\6.10.0\mingw_64
E:\qt\6.10.0\mingw_64\bin\windeployqt6.exe release\MouseWriterCpp.exe --no-translations --no-compiler-runtime