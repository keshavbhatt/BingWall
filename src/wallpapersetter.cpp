#include "wallpapersetter.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDebug>
#include <QEventLoop>
#include <QProcess>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>

// ---------------------------------------------------------------------------
// Helper: receives the async org.freedesktop.portal.Request::Response signal.
// Defined here (not in a header) so automoc picks it up via the .moc include
// at the bottom of this file.
// ---------------------------------------------------------------------------
class PortalResponseReceiver : public QObject {
  Q_OBJECT
public:
  explicit PortalResponseReceiver(QObject *parent = nullptr)
      : QObject(parent) {}

  uint responseCode = 2; ///< 0 = success, 1 = cancelled, 2 = other error
  bool received = false;

public slots:
  void onResponse(uint code, const QVariantMap & /*results*/) {
    responseCode = code;
    received = true;
    emit done();
  }

signals:
  void done();
};

WallpaperSetter::WallpaperSetter(QObject *parent) : QObject(parent) {}

bool WallpaperSetter::setWallpaper(const QString &localPath) {
  // Strip "file://" prefix if present
  QString cleanPath = localPath;
  if (cleanPath.startsWith(QStringLiteral("file://")))
    cleanPath = QUrl(cleanPath).toLocalFile();

  QString portalError;
  if (setViaPortal(cleanPath, &portalError))
    return true;

  qWarning() << "WALLPAPER: portal failed (" << portalError
             << "), falling back to gsettings";
  return setViaGsettings(cleanPath);
}

bool WallpaperSetter::setViaPortal(const QString &localPath,
                                   QString *errorOut) {
  QDBusInterface portal(QStringLiteral("org.freedesktop.portal.Desktop"),
                        QStringLiteral("/org/freedesktop/portal/desktop"),
                        QStringLiteral("org.freedesktop.portal.Wallpaper"),
                        QDBusConnection::sessionBus());

  if (!portal.isValid()) {
    if (errorOut)
      *errorOut = QStringLiteral("Wallpaper portal service not available.");
    return false;
  }

  QVariantMap options;
  options.insert(QStringLiteral("show-preview"), QVariant(false));
  options.insert(QStringLiteral("set-on"), QStringLiteral("background"));

  const QString uri = QUrl::fromLocalFile(localPath).toString();
  QDBusReply<QDBusObjectPath> reply =
      portal.call(QStringLiteral("SetWallpaperURI"), QString(), uri, options);

  if (!reply.isValid()) {
    if (errorOut)
      *errorOut = reply.error().message();
    return false;
  }

  // The portal is async: SetWallpaperURI returns a request handle immediately
  // and later emits Response(code, results) on that handle once the request
  // has been handled (possibly after a permission dialog). We must wait for
  // that signal before we know whether the wallpaper was actually applied.
  const QString requestPath = reply.value().path();

  PortalResponseReceiver receiver;
  const bool connected = QDBusConnection::sessionBus().connect(
      QString(), // match any sender
      requestPath, QStringLiteral("org.freedesktop.portal.Request"),
      QStringLiteral("Response"), &receiver,
      SLOT(onResponse(uint, QVariantMap)));

  if (!connected) {
    if (errorOut)
      *errorOut =
          QStringLiteral("Failed to subscribe to portal response signal.");
    return false;
  }

  // Spin a local event loop while waiting. 60 s timeout lets the user take
  // their time with a permission dialog without blocking the process forever.
  QEventLoop loop;
  QTimer timeoutTimer;
  timeoutTimer.setSingleShot(true);
  connect(&receiver, &PortalResponseReceiver::done, &loop, &QEventLoop::quit);
  connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
  timeoutTimer.start(60000);
  loop.exec();

  QDBusConnection::sessionBus().disconnect(
      QString(), requestPath, QStringLiteral("org.freedesktop.portal.Request"),
      QStringLiteral("Response"), &receiver,
      SLOT(onResponse(uint, QVariantMap)));

  if (!receiver.received) {
    if (errorOut)
      *errorOut = QStringLiteral("Portal response timed out.");
    return false;
  }

  if (receiver.responseCode != 0) {
    if (errorOut) {
      *errorOut = receiver.responseCode == 1
                      ? QStringLiteral("User cancelled the wallpaper dialog.")
                      : QStringLiteral("Portal returned error code: ") +
                            QString::number(receiver.responseCode);
    }
    return false;
  }

  return true;
}

bool WallpaperSetter::setViaGsettings(const QString &localPath) {
  QProcess gsettings;
  gsettings.start(QStringLiteral("gsettings"),
                  {QStringLiteral("get"),
                   QStringLiteral("org.gnome.desktop.interface"),
                   QStringLiteral("color-scheme")});
  gsettings.waitForFinished();
  const QString colorScheme(gsettings.readAllStandardOutput().trimmed());

  const QString pictureUri = colorScheme == QStringLiteral("'prefer-dark'")
                                 ? QStringLiteral("picture-uri-dark")
                                 : QStringLiteral("picture-uri");

  gsettings.start(QStringLiteral("gsettings"),
                  {QStringLiteral("set"),
                   QStringLiteral("org.gnome.desktop.background"), pictureUri,
                   QStringLiteral("file://") + localPath});
  if (!gsettings.waitForFinished() || gsettings.exitCode() != 0) {
    qWarning() << "GSETTINGS:Process failed:" << gsettings.errorString();
    return false;
  }
  qDebug() << "GSETTINGS:Process success:";
  return true;
}

// Required so automoc processes Q_OBJECT classes defined above in this .cpp
#include "wallpapersetter.moc"
