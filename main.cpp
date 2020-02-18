#include "mainwindow.h"
#include <QApplication>
#include "darkstyle.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("BingWall");
    QApplication::setOrganizationName("org.keshavnrj.ubuntu");

    a.setStyle(new DarkStyle);
    MainWindow w;
    w.show();

    return a.exec();
}
