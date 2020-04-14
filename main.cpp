#include "mainwindow.h"
#include <QApplication>
#include "darkstyle.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("BingWall");
    QApplication::setOrganizationName("org.keshavnrj.ubuntu");
    a.setStyle(new DarkStyle);
    MainWindow w;


    QStringList argsList = a.arguments();
    if(argsList.contains("--set",Qt::CaseSensitive)){
        //set wallpaper of the day in hidden mode, usually used when app is set to run on startup
        //procedure:
        //hide mainwindow, load wallpapers, if loaded, select first and set it as wallpaper,quit the app
        w.setWallpaperOfTheDay();
    }else{
        w.show();
    }

    return a.exec();
}
