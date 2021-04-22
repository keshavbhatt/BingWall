#ifndef REMOTEPIXMAPLABEL2_H
#define REMOTEPIXMAPLABEL2_H

#include <QObject>
#include <QLabel>
#include <QtNetwork>
#include <QPushButton>

class RemotePixmapLabel2 : public QLabel
{
    Q_OBJECT
signals:
    void pixmapLoaded(QByteArray data);
    void pixmapLoading();

public:
    RemotePixmapLabel2(QWidget *parent = 0);
    ~RemotePixmapLabel2();

    void init(QNetworkAccessManager *manager, const QString &url, QString fallbackUrlStr = "");

private slots:
    void pixmapReceived();
    void showRetry(const QString urlStr);
    void deleteButtons();
    void setRemotePixmap(const QString &url);
private:
    QString fallbackUrlStr;
    QNetworkAccessManager *networkManager_ = nullptr;
    QNetworkReply *reply = nullptr;



};

#endif // REMOTEPIXMAPLABEL2_H
