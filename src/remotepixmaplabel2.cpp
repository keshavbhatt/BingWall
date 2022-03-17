#include "remotepixmaplabel2.h"
#include <QDebug>

QString reloadIcon64 ="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";

RemotePixmapLabel2::RemotePixmapLabel2(QWidget *parent)
    : QLabel(parent)
{

}

RemotePixmapLabel2::~RemotePixmapLabel2()
{
    deleteButtons();
}

void RemotePixmapLabel2::init(QNetworkAccessManager *manager, const QString &url, QString fallbackUrlStr)
{
    //set fallback only on init
    if(this->fallbackUrlStr.isEmpty()){
        this->fallbackUrlStr = fallbackUrlStr;
    }

    this->networkManager_ = manager;
    setRemotePixmap(url);
}

void RemotePixmapLabel2::setRemotePixmap(const QString& url)
{
    deleteButtons();
    if(networkManager_ == nullptr)
        return;

    QNetworkRequest req(url);
    reply = this->networkManager_->get(req);
    connect(reply,SIGNAL(finished()),this,SLOT(pixmapReceived()));
    emit pixmapLoading();
}

void RemotePixmapLabel2::pixmapReceived()
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << Q_FUNC_INFO << "pixmap receiving error" << reply->error();
        if(this->fallbackUrlStr.isEmpty())
            setRemotePixmap(fallbackUrlStr);
        if(reply->request().url().isEmpty()==false)
            showRetry(reply->request().url().toString());
    }else{
        const QByteArray data(reply->readAll());
        if (data.size() != 0){
            QPixmap pixmap;
            pixmap.loadFromData(data);
            setPixmap(pixmap.scaled(this->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
            //check for currupt pixmap
            if(pixmap.isNull()==false){
                emit pixmapLoaded(data);
                networkManager_ = nullptr;
            }else{
                qDebug() << Q_FUNC_INFO << "pixmap currupt";
            }
        }else{
            qDebug() << Q_FUNC_INFO << "network okay, but received pixmap looks like nothing";
        }
    }
    reply->deleteLater();
}

void RemotePixmapLabel2::showRetry(const QString urlStr)
{
    deleteButtons();
    QByteArray data = QByteArray::fromBase64(reloadIcon64.toLatin1());
    QImage img = QImage::fromData(data,"PNG");
    QPushButton *retryBtn = new QPushButton(tr(""),this);
    retryBtn->setIcon(QIcon(QPixmap::fromImage(img)));
    retryBtn->setObjectName("retry");
    connect(retryBtn,&QPushButton::clicked,[=](){
       setRemotePixmap(urlStr);
    });
    retryBtn->show();
    retryBtn->raise();
    retryBtn->move(this->rect().center()-retryBtn->rect().center());
}

void RemotePixmapLabel2::deleteButtons()
{
    foreach (auto &btn, this->findChildren<QPushButton *>("retry")) {
        btn->hide();
        delete btn;
    }
}
