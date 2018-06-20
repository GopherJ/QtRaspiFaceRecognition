#include "mainwindow.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MainWindow w;
    //w.show();

    return a.exec();
}
