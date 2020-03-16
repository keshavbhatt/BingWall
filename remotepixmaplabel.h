#ifndef REMOTEPIXMAPLABEL_H
#define REMOTEPIXMAPLABEL_H


#include <QLabel>
#include <QtNetwork>

class RemotePixmapLabel: public QLabel
{
    Q_OBJECT
public:
    RemotePixmapLabel(QWidget* parent = 0) :
        QLabel(parent), networkManager_(0)
    {}
    RemotePixmapLabel(const QString& text, QWidget* parent = 0) :
        QLabel(text, parent), networkManager_(0)
    {
    }
public slots:
    void setRemotePixmap(const QString&  url , QNetworkDiskCache *diskCache);
private slots:
    void pixmapReceived(QNetworkReply*);
private:
    QNetworkAccessManager* networkManager_;
};

#endif // REMOTEPIXMAPLABEL_H
