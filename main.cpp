#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication Beijing_IHE_Navigation(argc, argv);
    QPixmap startpng(":/icon/icon/splash.png");
    QSplashScreen splash(startpng);
    splash.show();
    MainWindow mainwindow;
    mainwindow.show();
    splash.finish(&mainwindow);
    return Beijing_IHE_Navigation.exec();
}
