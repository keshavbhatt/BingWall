#ifndef WALLPAPERSETTER_H
#define WALLPAPERSETTER_H

#include <QObject>
#include <QString>

/**
 * @brief Applies a desktop wallpaper.
 *
 * Tries the XDG desktop portal (org.freedesktop.portal.Wallpaper) first —
 * the only mechanism that works from inside a strict sandbox and on any
 * desktop shipping a portal backend (GNOME, KDE, ...) — and falls back to
 * the historical gsettings (GNOME) approach when the portal is unavailable
 * or the request fails.
 */
class WallpaperSetter : public QObject {
  Q_OBJECT

public:
  explicit WallpaperSetter(QObject *parent = nullptr);

  /// Apply the image at @p localPath as the desktop wallpaper. Blocks until
  /// the portal responds (or times out). Returns true on success.
  bool setWallpaper(const QString &localPath);

private:
  bool setViaPortal(const QString &localPath, QString *errorOut);
  bool setViaGsettings(const QString &localPath);
};

#endif // WALLPAPERSETTER_H
