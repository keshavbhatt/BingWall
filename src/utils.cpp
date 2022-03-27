#include "utils.h"
#include <QDateTime>
#include <QDesktopServices>
#include <QProcess>
#include <QRegularExpression>
#include <QUuid>

utils::utils(QObject *parent) : QObject(parent) {}

utils::~utils() { this->deleteLater(); }

// calculate dir size
quint64 utils::dir_size(const QString &directory) {
  quint64 sizex = 0;
  QFileInfo str_info(directory);
  if (str_info.isDir()) {
    QDir dir(directory);
    QFileInfoList list =
        dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden |
                          QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (int i = 0; i < list.size(); ++i) {
      QFileInfo fileInfo = list.at(i);
      if (fileInfo.isDir()) {
        sizex += dir_size(fileInfo.absoluteFilePath());
      } else {
        sizex += fileInfo.size();
      }
    }
  }
  return sizex;
}

// get the size of cache folder in human readble format
QString utils::refreshCacheSize(const QString cache_dir) {
  qint64 cache_size = dir_size(cache_dir);
  QString cache_unit;
  if (cache_size > 1024 * 1024 * 1024) {
    cache_size = cache_size / (1024 * 1024 * 1024);
    cache_unit = " GB";
  }
  if (cache_size > 1024 * 1024) {
    cache_size = cache_size / (1024 * 1024);
    cache_unit = " MB";
  } else if (cache_size > 1024) {
    cache_size = cache_size / (1024);
    cache_unit = " kB";
  } else {
    cache_unit = " B";
  }
  return QString::number(cache_size) + cache_unit;
}

bool utils::delete_cache(const QString cache_dir) {
  bool deleted = QDir(cache_dir).removeRecursively();
  QDir(cache_dir).mkpath(cache_dir);
  return deleted;
}

// returns string with first letter capitalized
QString utils::toCamelCase(const QString &s) {
  QStringList parts = s.split(' ', QString::SkipEmptyParts);
  for (int i = 0; i < parts.size(); ++i)
    parts[i].replace(0, 1, parts[i][0].toUpper());
  return parts.join(" ");
}

QString utils::upperFirstChar(const QString &s) {
  QStringList parts = s.split(' ', QString::SkipEmptyParts);
  for (int i = 0; i < parts.size(); ++i) {
    if (i == 0) {
      parts[i].replace(0, 1, parts[i][0].toUpper());
    }
  }
  return parts.join(" ");
}

QString utils::generateRandomId(int length) {

  QString str = QUuid::createUuid().toString();
  str.remove(QRegularExpression("{|}|-"));
  if (str.length() < length) {
    while (str.length() != length) {
      int required_char = length - str.length();
      str = str + str.append(genRand(required_char));
    }
  }
  if (str.length() > length) {
    while (str.length() != length) {
      str = str.remove(str.length() - 1, 1);
    }
  }
  return str;
}

QString utils::genRand(int length) {
  QDateTime cd = QDateTime::currentDateTime();
  const QString possibleCharacters(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" +
      QString::number(cd.currentMSecsSinceEpoch())
          .remove(QRegExp("[^a-zA-Z\\d\\s]")));

  const int randomStringLength = length;
  QString randomString;
  qsrand(cd.toTime_t());
  for (int i = 0; i < randomStringLength; ++i) {
    int index = qrand() % possibleCharacters.length();
    QChar nextChar = possibleCharacters.at(index);
    randomString.append(nextChar);
  }
  randomString = randomString.trimmed().simplified();
  return randomString;
}

QString utils::convertSectoDay(qint64 secs) {
  int day = secs / (24 * 3600);

  secs = secs % (24 * 3600);
  int hour = secs / 3600;

  secs %= 3600;
  int minutes = secs / 60;

  secs %= 60;
  int seconds = secs;

  QString days = QString::number(day) + " " + "days " + QString::number(hour) +
                 " " + "hours " + QString::number(minutes) + " " + "minutes " +
                 QString::number(seconds) + " " + "seconds ";
  return days;
}

// static on demand path maker
QString utils::returnPath(QString pathname) {
  QString _data_path =
      QStandardPaths::writableLocation(QStandardPaths::DataLocation);
  if (!QDir(_data_path + "/" + pathname).exists()) {
    QDir d(_data_path + "/" + pathname);
    d.mkpath(_data_path + "/" + pathname);
  }
  return _data_path + "/" + pathname + "/";
}

QString utils::htmlToPlainText(QString str) {
  QString out;
  QTextDocument text;
  text.setHtml(str);
  out = text.toPlainText();
  text.deleteLater();
  return out.replace("&amp;", "&")
      .replace("&gt;", ">")
      .replace("&lt;", "<")
      .replace("&#39;", "'");
}

bool utils::splitString(const QString &str, int m, QStringList &list) {
  if (m < 1)
    return false;
  QStringList words = str.split(" ");
  while (words.isEmpty() == false) {
    QString strPart;
    if (QString(words.join(" ")).length() > m) {
      for (int i = 0; i < words.count(); i++) {
        if (strPart.count() < m) {
          strPart.append(words.at(i) + " ");
          words.removeAt(i);
          --i;
        }
      }
    } else if (QString(words.join(" ")).length() < m) {
      strPart.append(words.join(" "));
      words.clear();
    }
    list.append(strPart);
  }
  return true;
}

QString utils::removeNodeColor(QString input) {
  QStringList colors;
  QString output;
  colors << "\x1b[0m"
         << "\x1b[1m"
         << "\x1b[2m"
         << "\x1b[4m"
         << "\x1b[5m"
         << "\x1b[7m"
         << "\x1b[8m"
         << "\x1b[30m"
         << "\x1b[31m"
         << "\x1b[32m"
         << "\x1b[33m"
         << "\x1b[34m"
         << "\x1b[35m"
         << "\x1b[36m"
         << "\x1b[37m"
         << "\x1b[40m"
         << "\x1b[41m"
         << "\x1b[42m"
         << "\x1b[43m"
         << "\x1b[44m"
         << "\x1b[45m"
         << "\x1b[46m"
         << "\x1b[47m"
         << "\x1b[39m"
         << "\x1b[22m ";
  foreach (QString str, colors) { output = input.replace(str, " "); }
  return output;
}

QString utils::humanReadableSize(double bytes) {
  double cache_size = static_cast<double>(bytes);
  QString cache_unit;
  if (cache_size > 1024 * 1024 * 1024) {
    cache_size = cache_size / (1024 * 1024 * 1024);
    cache_unit = " GB";
  } else if (cache_size > 1024 * 1024) {
    cache_size = cache_size / (1024 * 1024);
    cache_unit = " MB";
  } else if (cache_size > 1024) {
    cache_size = cache_size / (1024);
    cache_unit = " kB";
  } else {
    cache_unit = " B";
  }
  return QString::number(cache_size, 'f', 2) + cache_unit;
}

bool utils::isMedia(QString fileName) {
  bool isGood = false;
  if (isVideo(fileName) || isAudio(fileName)) {
    isGood = true;
  }
  return isGood;
}

bool utils::isVideo(QString filename) {
  QStringList video = {".3g2", ".3gp", ".aaf",  ".asf",  ".avchd", ".avi",
                       ".drc", ".flv", ".m2v",  ".m4p",  ".m4v",   ".mkv",
                       ".mng", ".mov", ".mp2",  ".mp4",  ".mpe",   ".mpeg",
                       ".mpg", ".mpv", ".mxf",  ".nsv",  ".ogg",   ".ogv",
                       ".ogm", ".qt",  ".rm",   ".rmvb", ".roq",   ".srt",
                       ".svi", ".vob", ".webm", ".wmv",  ".yuv"};

  bool isGood = false;
  foreach (QString filter, video) {
    if (filename.contains(filter, Qt::CaseInsensitive)) {
      isGood = true;
      break;
    }
  }
  return isGood;
}

bool utils::isAudio(QString filename) {
  QStringList audio = {".aac", ".aiff", ".ape", ".au",  ".flac", ".gsm", ".it",
                       ".m3u", ".m4a",  ".mid", ".mod", ".mp3",  ".mpa", ".pls",
                       ".ra",  ".s3m",  ".sid", ".wav", ".wma",  ".xm"};

  bool isGood = false;
  foreach (QString filter, audio) {
    if (filename.contains(filter, Qt::CaseInsensitive)) {
      isGood = true;
      break;
    }
  }
  return isGood;
}

bool utils::isImage(QString filename) {
  QStringList images = {".3dm",  ".3ds", ".max", ".bmp", ".dds", ".gif", ".jpg",
                        ".jpeg", ".png", ".psd", ".xcf", ".tga", ".thm", ".tif",
                        ".tiff", ".yuv", ".ai",  ".eps", ".ps",  ".svg", ".dwg",
                        ".dxf",  ".gpx", ".kml", ".kmz", ".webp"};

  bool isGood = false;
  foreach (QString filter, images) {
    if (filename.contains(filter, Qt::CaseInsensitive)) {
      isGood = true;
      break;
    }
  }
  return isGood;
}

QString utils::cleanString(QString input) {
  QStringList sp_chars = {"\\", ",", "?", "!", "`", "_", "#",
                          "$",  "%", "^", "&", "*", ";", ":",
                          "/",  ">", "<", "~", "=", "+", "⭐"};
  foreach (QString sp_char, sp_chars) { input.replace(sp_char, " "); }
  return input;
}

QString utils::randomIpV6() {
  QDateTime cd = QDateTime::currentDateTime();
  const QString possibleCharacters("abcdef0123456789" +
                                   QString::number(cd.currentMSecsSinceEpoch())
                                       .remove(QRegExp("[^a-zA-Z\\d\\s]")));
  const int randomStringLength = 28;
  QString randomString;
  qsrand(cd.toTime_t());
  for (int i = 0; i < randomStringLength; ++i) {
    int index = qrand() % possibleCharacters.length();
    QChar nextChar = possibleCharacters.at(index);
    if (i == 4 || i == 8 || i == 12 || i == 16 || i == 20 || i == 24) {
      randomString.append(":");
    }
    randomString.append(nextChar);
  }
  return randomString.trimmed().simplified().remove(" ");
}

bool utils::desktopOpenUrl(const QString str){
    QProcess *xdg_open = new QProcess(this);
    xdg_open->start("xdg-open", QStringList() << str);
    if (xdg_open->waitForStarted(1000) == false) {
      // try using QdesktopServices
      bool opened = QDesktopServices::openUrl(QUrl(str));
      if (opened == false) {
        qWarning() << "failed to open url" << str;
      }
    }
    connect(
        xdg_open,
        static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
            &QProcess::finished),
        [this, xdg_open](int exitCode, QProcess::ExitStatus exitStatus) {
          Q_UNUSED(exitCode);
          Q_UNUSED(exitStatus);
          xdg_open->close();
          xdg_open->deleteLater();
        });
}
