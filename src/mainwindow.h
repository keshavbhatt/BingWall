#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QListWidgetItem>
#include <QSettings>

#include "request.h"
#include "waitingspinnerwidget.h"
#include "utils.h"

#include "ui_listitem_ui.h"
#include "ui_action_ui.h"
#include "ui_settings.h"
#include "ui_downloaded.h"
#include "ui_downloaded_option.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void setWallpaperOfTheDay();
signals:
    void wallpaperSet();
    void wallpaperListLoaded(bool containsWallpapers);

protected slots:
    bool eventFilter(QObject *obj, QEvent *event);
private slots:
    void load_wallpapers();
    void parse_json_reply(const QString reply);
    void refreshLoader();
    void load_data_into_view(QList<QStringList> data);
    void on_wallpaperList_itemClicked(QListWidgetItem *item);


    void on_left_clicked();

    void on_right_clicked();
    void updateNavigationButtons();
    void on_wallpaperList_currentRowChanged(int currentRow);

    void setWallpaper();
    QString returnPath(QString pathname);
    void init_request(QNetworkAccessManager *nm);
    void apply_wallpaper(QString filePath);
    void on_setting_clicked();

    void location_changed(QString newLocation);
    void on_downloaded_clicked();

    void load_downloaded_wallpapers();
    void show_option_for_downloaded(QObject *obj);
    void hide_option_for_downloaded(QObject *obj);
    void delete_wallpaper(QString filePath);
    void on_wonderwall_clicked();

    void check_for_startup();
    void launch_on_startup_toggled(bool arg1);
    void run_onstartup(bool enabled);
    bool saveWall(const QString &path, QString imagePath);
    void updateAbout();
private:
    Ui::MainWindow *ui;
    Ui::thumbWidget _ui_listitem;
    Ui::actionWidget _ui_action;
    Ui::setting_widget _ui_settings;
    Ui::downloaded_widget _ui_downloaded;
    Ui::downloaded_option_ui _ui_downloaded_option;
    Request * _request = nullptr;
    QLabel *_wall_view = nullptr;
    WaitingSpinnerWidget *_loader = nullptr;
    QTimer *_loader_timer = nullptr;
    QString _data_path;
    QString _currentUrl;
    bool _isDownloading = false;
    QWidget *setting_widget = nullptr;
    QString _location = "us";
    QStringList _locations ;
    QWidget *downloaded_widget = nullptr;

    QSettings settings;

    QNetworkAccessManager *networkManager_ = nullptr;

    int rows,cols;
    bool downloded_loaded = false;
    QString currentWallInfo = "";

    QString htmlToPlainText(QString str){
        QString out;
            QTextDocument text;
            text.setHtml(str);
            out = text.toPlainText();
            text.deleteLater();
        return out .replace("\\\"","'")
                .replace("&amp;","&")
                .replace("&gt;",">")
                .replace("&lt;","<")
                .replace("&#39;","'");
    }



    void LoadCover(const QUrl &avatarUrl,QLabel &lable)
    {
       QString cache_path =  QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

       QNetworkAccessManager manager;
       manager.setParent(this);
       QNetworkDiskCache* diskCache = new QNetworkDiskCache(this);
       diskCache->setCacheDirectory(cache_path);
       manager.setCache(diskCache);

       QEventLoop loop;
       loop.setParent(this);
       QNetworkReply *reply = manager.get(QNetworkRequest(avatarUrl));
       QObject::connect(reply, &QNetworkReply::finished, &loop, [&reply, this,&loop ,&lable](){
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray jpegData = reply->readAll();
            QPixmap pixmap;
            pixmap.loadFromData(jpegData);
            if (!pixmap.isNull())
            {
                lable.setPixmap(pixmap.scaled(lable.size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
            }
        }
        reply->deleteLater();
        loop.quit();
      });
        diskCache->deleteLater();
        manager.deleteLater();
      loop.exec();
    }
};

#endif // MAINWINDOW_H
