#include "request.h"

Request::Request(QObject *parent) : QObject(parent) {
  setParent(parent);
  _cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

Request::~Request() { this->deleteLater(); }

void Request::get(const QUrl url) {
  QNetworkAccessManager *m_netwManager = new QNetworkAccessManager(this);
  QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
  diskCache->setCacheDirectory(_cache_path);
  m_netwManager->setCache(diskCache);
  connect(m_netwManager, &QNetworkAccessManager::finished,
          [=](QNetworkReply *rep) {
            if (rep->error() == QNetworkReply::NoError) {
              QString repStr = rep->readAll();
              emit requestFinished(repStr);
            } else {
              emit downloadError(rep->errorString());
            }
            rep->deleteLater();
            m_netwManager->deleteLater();
          });
  QNetworkRequest request(url);
  m_netwManager->get(request);
  emit requestStarted();
}

void Request::download_wallpaper(const QUrl url) {
  QNetworkAccessManager *m_netwManager = new QNetworkAccessManager(this);
  QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
  diskCache->setCacheDirectory(_cache_path);
  m_netwManager->setCache(diskCache);
  connect(m_netwManager, &QNetworkAccessManager::finished,
          [=](QNetworkReply *rep) {
            if (rep->error() == QNetworkReply::NoError) {
              emit downloadFinished(*rep);
            } else {
              emit downloadError(rep->errorString());
            }
          });
  QNetworkRequest request(url);
  QString imageFileName = url.toString().split("/").last();
  request.setAttribute(QNetworkRequest::User, imageFileName);

  QNetworkReply *reply = m_netwManager->get(request);
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
