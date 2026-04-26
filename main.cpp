#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMessageBox::information(nullptr, "Test", "App is starting!");
    MainWindow w;
    w.show();
    QMessageBox::information(nullptr, "Test", "Window shown!");
    return a.exec();
}