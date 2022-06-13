#include "request.h"

Request::Request(QObject *parent, QNetworkAccessManager *nm) : QObject(parent) {
  this->networkAccessManager = nm;
}

Request::~Request() { this->deleteLater(); }

void Request::get(const QUrl url) {
  QNetworkRequest request(url);
  QNetworkReply *reply = networkAccessManager->get(request);
  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      QString repStr = reply->readAll();
      emit requestFinished(repStr);
    } else {
      emit downloadError(reply->errorString());
    }
    reply->deleteLater();
  });
  emit requestStarted();
}

void Request::download_wallpaper(const QUrl url) {

  QNetworkRequest request(url);
  QString imageFileName = url.toString().split("/").last();
  request.setAttribute(QNetworkRequest::User, imageFileName);

  QNetworkReply *reply = networkAccessManager->get(request);
  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      emit downloadFinished(*reply);
    } else {
      emit downloadError(reply->errorString());
    }
    reply->deleteLater();
  });
  connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this,
          SLOT(downloadProgress(qint64, qint64)));
  emit downloadStarted();
}

void Request::downloadProgress(qint64 got, qint64 tot) {
  double downloaded_Size = (double)got;
  double total_Size = (double)tot;
  double progress = (downloaded_Size / total_Size) * 100;
  int intProgress = static_cast<int>(progress);
  emit _downloadProgress(intProgress);
}
