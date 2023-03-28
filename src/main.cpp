#include "mainwindow.h"

#include <QApplication>

int main(int argc,char *argv[]) {
    QApplication appMain(argc,argv);
    MainWindow   winMain;
    winMain.show();
    return appMain.exec();
}
