#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "gridlayoututil.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QGraphicsOpacityEffect>
#include <QHoverEvent>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPainter>
#include <QScreen>
#include <QTextBrowser>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  setWindowTitle(QApplication::applicationName());
  setWindowIcon(QIcon(":/resources/binfwall_128.png"));

  networkManager_ = new QNetworkAccessManager(this);
  QString cache_path =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
  diskCache->setCacheDirectory(cache_path);
  networkManager_->setCache(diskCache);

  init_request(networkManager_);
  _data_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

  // wall_view is the child of monitor
  _wall_view = new QLabel(ui->monitor);
  _wall_view->setAlignment(Qt::AlignCenter);
  _wall_view->setPixmap(QPixmap(":/resources/180.jpg"));
  _wall_view->setGeometry(12, 26, 319, 180);
  _wall_view->show();
  _wall_view->installEventFilter(this);
  _wall_view->setAttribute(Qt::WA_Hover, true);
  _wall_view->setMouseTracking(true);

  // _ui_action is the child of wall_view
  QWidget *actionWidget = new QWidget(_wall_view);
  actionWidget->setObjectName("actions_view");
  _ui_action.setupUi(actionWidget);
  _ui_action.progressBar->hide();
  //    connect(_ui_action.set,SIGNAL(clicked()),this,SLOT(setWallpaper()));
  connect(_ui_action.set, &QPushButton::clicked, [=]() {
    // update currentWallInfo before setting wallpaper
    QWidget *listwidget = ui->wallpaperList->itemWidget(
        ui->wallpaperList->item(ui->wallpaperList->currentRow()));
    if (listwidget != nullptr) {
      currentWallInfo = listwidget->toolTip();
    }
    setWallpaper();
  });
  actionWidget->setGeometry(_wall_view->rect());
  actionWidget->hide();

  // loader is the child of wall_view
  _loader = new WaitingSpinnerWidget(_wall_view, true, false);
  _loader->setRoundness(70.0);
  _loader->setMinimumTrailOpacity(15.0);
  _loader->setTrailFadePercentage(70.0);
  _loader->setNumberOfLines(40);
  _loader->setLineLength(20);
  _loader->setLineWidth(2);
  _loader->setInnerRadius(2);
  _loader->setRevolutionsPerSecond(3);
  _loader->setColor(QColor(159, 160, 164));

  _loader_timer = new QTimer(this);
  connect(_loader_timer, &QTimer::timeout, [=]() {
    if (_loader->color() == QColor(159, 160, 164)) {
      _loader->setColor(QColor(107, 164, 231));
    } else if (_loader->color() == QColor(107, 164, 231)) {
      _loader->setColor(QColor(159, 160, 164));
    }
  });

  QIcon left;
  left.addPixmap(QPixmap(":/resources/arrow-left-circle-line.png"),
                 QIcon::Normal);
  left.addPixmap(QPixmap(":/resources/disabled-arrow-left-circle-line.png"),
                 QIcon::Disabled);
  ui->left->setIcon(left);

  QIcon right;
  right.addPixmap(QPixmap(":/resources/arrow-right-circle-line.png"),
                  QIcon::Normal);
  right.addPixmap(QPixmap(":/resources/disabled-arrow-right-circle-line.png"),
                  QIcon::Disabled);
  ui->right->setIcon(right);

  setting_widget = new QWidget(this);
  setting_widget->setWindowFlags(Qt::Dialog);
  _ui_settings.setupUi(setting_widget);
  setting_widget->setWindowModality(Qt::NonModal);
  setting_widget->adjustSize();
  setting_widget->setWindowTitle(QApplication::applicationName() +
                                 " | Settings");

  updateAbout();

  downloaded_widget = new QWidget(this);
  downloaded_widget->setWindowFlags(Qt::Dialog);
  _ui_downloaded.setupUi(downloaded_widget);

  downloaded_widget->setWindowModality(Qt::NonModal);
  downloaded_widget->adjustSize();
  downloaded_widget->setWindowTitle(QApplication::applicationName() +
                                    " | Downloaded");
  connect(_ui_downloaded.reload, &controlButton::clicked, [=]() {
    downloded_loaded = false;
    load_downloaded_wallpapers();
  });

  connect(_ui_downloaded.open_download_dir, &controlButton::clicked,
          [=]() { utils::desktopOpenUrl(returnPath("downloaded")); });

  check_for_startup();

  connect(_ui_settings.startUp, SIGNAL(toggled(bool)), this,
          SLOT(launch_on_startup_toggled(bool)));

  _ui_settings.fullRes->setChecked(settings.value("fullRes", true).toBool());
  connect(_ui_settings.fullRes, &QCheckBox::toggled,
          [=](bool checked) { settings.setValue("fullRes", checked); });

  _ui_settings.watermark->setChecked(
      settings.value("watermark", false).toBool());
  connect(_ui_settings.watermark, &QCheckBox::toggled,
          [=](bool checked) { settings.setValue("watermark", checked); });

  _locations << "au"
             << "ca"
             << "cn"
             << "de"
             << "fr"
             << "in"
             << "it"
             << "jp"
             << "es"
             << "gb"
             << "us";

  connect(_ui_settings.locationCombo, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(location_changed(QString)));

  _ui_settings.locationCombo->setCurrentIndex(
      settings.value("location", "9").toInt());

  connect(_ui_settings.rate, &QPushButton::clicked,
          [=]() { QDesktopServices::openUrl(QUrl("snap://bing-wall")); });
  connect(_ui_settings.github, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://github.com/keshavbhatt/BingWall"));
  });
  connect(_ui_settings.donate, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://paypal.me/keshavnrj/4"));
  });
  connect(_ui_settings.moreApps, &QPushButton::clicked, [=]() {
    QDesktopServices::openUrl(QUrl("https://snapcraft.io/search?q=keshavnrj"));
  });
  connect(_ui_settings.downloads, &QPushButton::clicked,
          [=]() { utils::desktopOpenUrl(returnPath("downloaded")); });

  for (int i = 0; i < _ui_settings.locationCombo->count(); i++) {
    _ui_settings.locationCombo->setItemIcon(
        i, QIcon(":/resources/map-pin-line.png"));
    _ui_settings.locationCombo->setIconSize(QSize(22, 22));
  }

  QString btn_style_3 =
      "QPushButton{background-color:transparent ;border:0px;padding-top: 3px; "
      "padding-bottom: 3px;}"
      "QPushButton:disabled {padding-top: 3px; padding-bottom:3px;}"
      "QPushButton:pressed {padding-bottom:0px;border:0px;}"
      "QPushButton:hover "
      "{padding-bottom:1px;background-color:transparent;border:0px;}";

  foreach (QPushButton *btn, this->findChildren<QPushButton *>()) {
    btn->setStyleSheet(btn_style_3);
  }

  ui->wonderwall->installEventFilter(this);
  updateNavigationButtons();
  refreshLoader();
  load_wallpapers();

  ui->wallpaperList->setDragEnabled(false);
  ui->wallpaperList->setDragDropMode(QAbstractItemView::NoDragDrop);
}

void MainWindow::location_changed(QString newLocation) {
  setWindowTitle(QApplication::applicationName() + " | " + newLocation);
  int index = _ui_settings.locationCombo->currentIndex();
  settings.setValue("location", index);
  _location = _locations.at(index);
  _currentUrl = "";
  _isDownloading = false;
  ui->wallpaperList->clear();
  load_wallpapers();
}

// on demand path maker
QString MainWindow::returnPath(QString pathname) {
  if (!QDir(_data_path + "/" + pathname).exists()) {
    QDir d(_data_path + "/" + pathname);
    d.mkpath(_data_path + "/" + pathname);
  }
  return _data_path + "/" + pathname + "/";
}

void MainWindow::setWallpaper() {
  if (settings.value("fullRes", false).toBool() == true) {
    _currentUrl = _currentUrl.remove("_1080");
  }
  _request->download_wallpaper(QUrl(_currentUrl));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj == _wall_view && !_currentUrl.isEmpty()) {
    const QHoverEvent *const he = static_cast<const QHoverEvent *>(event);
    QWidget *actionWidget = _wall_view->findChild<QWidget *>("actions_view");
    switch (he->type()) {
    case QEvent::HoverEnter:
      actionWidget->show();
      break;
    case QEvent::HoverLeave:
      if (!_isDownloading)
        actionWidget->hide();
      break;
    default:
      break;
    }
  }

  if (obj == ui->wonderwall) {
    const QHoverEvent *const he = static_cast<const QHoverEvent *>(event);
    switch (he->type()) {
    case QEvent::HoverEnter:
      ui->wonderwall->setIcon(QIcon(":/resources/wonderwall2.png"));
      break;
    case QEvent::HoverLeave:
      ui->wonderwall->setIcon(QIcon(":/resources/wonderwall.png"));
      break;
    default:
      break;
    }
  }
  if (obj->objectName().contains("downloaded_wall-")) {
    const QHoverEvent *const he = static_cast<const QHoverEvent *>(event);
    switch (he->type()) {
    case QEvent::HoverEnter:
      show_option_for_downloaded(obj);
      break;
    case QEvent::HoverLeave:
      if (!_isDownloading)
        hide_option_for_downloaded(obj);

      break;
    default:
      break;
    }
  }
  return QMainWindow::eventFilter(obj, event);
}

void MainWindow::show_option_for_downloaded(QObject *obj) {
  QWidget *objWidget = qobject_cast<QWidget *>(obj);
  QWidget *option_widget =
      obj->findChild<QWidget *>("option_" + obj->objectName());
  if (option_widget == nullptr) {
    QWidget *option = new QWidget(objWidget);
    option->setObjectName("option_" + obj->objectName());
    _ui_downloaded_option.setupUi(option);
    connect(_ui_downloaded_option.set_button, &QPushButton::clicked, [=]() {
      QString fileName = returnPath("downloaded") +
                         obj->objectName().split("downloaded_wall-").last();
      QString fileNameWaterMarked =
          returnPath("downloaded_water_marked") +
          obj->objectName().split("downloaded_wall-").last();

      QFileInfo f(fileName);
      qDebug() << f.fileName();
      QFile detail(returnPath("details") + f.baseName());
      if (detail.open(QFile::ReadOnly))
        currentWallInfo = detail.readAll();
      detail.close();

      if (settings.value("watermark", false).toBool() == true) {
        if (saveWall(fileNameWaterMarked, fileName)) {
          apply_wallpaper(fileNameWaterMarked);
        }
      } else {
        apply_wallpaper(fileName);
      }
    });
    connect(_ui_downloaded_option.delete_button, &QPushButton::clicked, [=]() {
      delete_wallpaper(returnPath("downloaded") +
                       obj->objectName().split("downloaded_wall-").last());
      downloded_loaded = false;
      load_downloaded_wallpapers();
    });
    option->setGeometry(objWidget->rect());
    option->show();
  } else {
    option_widget->show();
  }
}
void MainWindow::hide_option_for_downloaded(QObject *obj) {
  QWidget *option_widget =
      obj->findChild<QWidget *>("option_" + obj->objectName());
  if (option_widget != nullptr) {
    option_widget->hide();
    option_widget->deleteLater();
  }
}

void MainWindow::refreshLoader() {
  if (_loader->isSpinning()) {
    _loader_timer->start(100);
  } else {
    _loader_timer->stop();
  }
}

void MainWindow::init_request(QNetworkAccessManager *nm) {
  _request = new Request(this, nm);
  connect(_request, &Request::requestStarted, [=]() {
    _loader->start();
    refreshLoader();
  });
  connect(_request, &Request::requestFinished, [=](QString reply) {
    _loader->stop();
    refreshLoader();
    parse_json_reply(reply);
  });

  // for downloading wallpaper
  connect(_request, &Request::downloadStarted, [=]() {
    _isDownloading = true;
    _ui_action.set->setEnabled(false);
    _ui_action.progressBar->show();
  });

  connect(_request, &Request::downloadFinished, [=](QNetworkReply &reply) {
    _isDownloading = false;
    _ui_action.progressBar->setMinimum(0);
    _ui_action.progressBar->setMaximum(0);
    _ui_action.progressBar->hide();
    _ui_action.set->setEnabled(true);

    QNetworkRequest request = reply.request();
    if (reply.error() == QNetworkReply::NoError) {
      QVariant v = request.attribute(QNetworkRequest::User);
      QString fileName = v.toString();
      // fix wrong filename bug caused due the presence of "?attachment" in
      // fileName
      fileName = fileName.remove("?attachment");
      qDebug() << "IMAGE:File name" << fileName;
      QFile file(returnPath("downloaded") + fileName);
      if (file.open(QFile::ReadWrite))
        file.write(reply.readAll());
      file.close();

      if (currentWallInfo.trimmed().isEmpty()) {
        currentWallInfo = "BingWall";
      }

      QFileInfo f(file);
      QFile detail(returnPath("details") + f.baseName());
      if (detail.open(QFile::ReadWrite))
        detail.write(currentWallInfo.toUtf8());
      detail.close();

      if (settings.value("watermark", false).toBool() == true) {
        if (saveWall(returnPath("downloaded_water_marked") + fileName,
                     file.fileName())) {
          apply_wallpaper(returnPath("downloaded_water_marked") + fileName);
        }
      } else {
        apply_wallpaper(file.fileName());
      }
      load_downloaded_wallpapers();
    }
    reply.deleteLater();
  });
  connect(_request, &Request::_downloadProgress, [=](int progress) {
    if (progress > 0) {
      _ui_action.progressBar->setMinimum(0);
      _ui_action.progressBar->setMaximum(100);
      _ui_action.progressBar->setValue(progress);
    }
  });
  connect(_request, &Request::downloadError, [=](QString errorString) {
    _isDownloading = false;
    _ui_action.progressBar->setMinimum(0);
    _ui_action.progressBar->setMaximum(0);
    _ui_action.progressBar->hide();
    _ui_action.set->setEnabled(true);
  });
}

bool MainWindow::saveWall(const QString &path, QString imagePath) {

  if (imagePath.split("/").last().contains(".") == false) {
    imagePath = imagePath + ".jpg";
    if (QFileInfo(imagePath).exists() == false)
      imagePath = imagePath + ".png";
  }

  QImage image(imagePath);
  QPainter p;
  if (!p.begin(&image))
    return false;

  p.setPen(QPen(QColor(255, 255, 255, 0)));
  QFont font = p.font();
  font.setPixelSize(22);
  font.setBold(true);
  p.setFont(font);
  if (currentWallInfo.trimmed().isEmpty()) {
    currentWallInfo = "BingWall";
  }
  QRect boundingRect;
  // draw fake text to calc bound rect
  p.drawText(QRect(image.rect().x() - 20, image.rect().y() - 20,
                   image.rect().width(), image.rect().height()),
             Qt::AlignBottom | Qt::AlignRight | Qt::TextWordWrap,
             htmlToPlainText(currentWallInfo), &boundingRect);

  p.fillRect(QRectF(boundingRect.x() - 5, boundingRect.y() - 5,
                    boundingRect.width() + 10, boundingRect.height() + 10),
             QBrush(QColor(0, 0, 0, 125)));
  p.setPen(QPen(QColor(255, 255, 255, 125)));
  p.drawText(image.rect().x() - 20, image.rect().y() - 20, image.rect().width(),
             image.rect().height(),
             Qt::AlignBottom | Qt::AlignRight | Qt::TextWordWrap,
             htmlToPlainText(currentWallInfo), &boundingRect);
  p.end();
  if (path.split("/").last().contains(".")) {
    return image.save(path);
  } else {
    return image.save(path + "." + QFileInfo(imagePath).suffix());
  }
}

void MainWindow::delete_wallpaper(QString filePath) {
  // little hack to check if file was passed with exetension suffix.
  // if not try to guess the file name by apending possible suffixes
  if (filePath.split("/").last().contains(".") == false) {
    filePath = filePath + ".jpg";
    if (QFileInfo(filePath).exists() == false)
      filePath = filePath + ".png";
  }

  QFile file(filePath);
  QFileInfo f(file);
  file.remove();
  file.deleteLater();

  QFile fileWaterMark(
      filePath.replace("/downloaded/", "/downloaded_water_marked/"));
  fileWaterMark.remove();
  fileWaterMark.deleteLater();

  QFile detail(returnPath("details") + f.baseName());
  detail.remove();
  detail.deleteLater();
}

void MainWindow::apply_wallpaper(QString filePath) {
  // little hack to check if file was passed with exetension suffix.
  // if not try to guess the file name by apending possible suffixes
  if (filePath.split("/").last().contains(".") == false) {
    filePath = filePath + ".jpg";
    if (QFileInfo(filePath).exists() == false)
      filePath = filePath + ".png";
  }

  QProcess *gsettings = new QProcess(this);
  gsettings->closeReadChannel(QProcess::StandardOutput);
  gsettings->closeReadChannel(QProcess::StandardError);
  gsettings->closeWriteChannel();

  gsettings->start("gsettings", QStringList() << "get"
                                              << "org.gnome.desktop.interface"
                                              << "color-scheme");
  gsettings->waitForFinished();
  QString colorScheme(gsettings->readAllStandardOutput().trimmed());
  QString picture_uri;

  if (colorScheme == "'prefer-dark'") {
    picture_uri = "picture-uri-dark";
  } else {
    picture_uri = "picture-uri";
  }

  gsettings->start("gsettings", QStringList()
                                    << "set"
                                    << "org.gnome.desktop.background"
                                    << picture_uri << "file://" + filePath);
  if (!gsettings->waitForFinished()) {
    qDebug() << "GSETTINGS:Process failed:" << gsettings->errorString();
  } else {
    qDebug() << "GSETTINGS:Process success:";
    emit wallpaperSet();
  }
  gsettings->deleteLater();
}

void MainWindow::load_wallpapers() {
  _request->get(QUrl("https://peapix.com/bing/feed?country=" + _location));
}

void MainWindow::parse_json_reply(const QString reply) {
  QJsonDocument jsonResponse = QJsonDocument::fromJson(reply.toUtf8());
  if (jsonResponse.isEmpty()) {
    qDebug()
        << "API:"
        << "Empty response returned from API call. Please report to developer.";
    return;
  }
  QList<QStringList> data;
  QJsonArray jsonArray = jsonResponse.array();
  foreach (const QJsonValue &val, jsonArray) {
    QJsonObject object = val.toObject();
    QString title, copyright, fullUrl, thumbUrl, date;
    title = object.value("title").toString();
    copyright = object.value("copyright").toString();
    fullUrl = object.value("fullUrl").toString();
    thumbUrl = object.value("thumbUrl").toString();
    date = object.value("date").toString();

    QStringList item = {title, copyright, fullUrl, thumbUrl, date};
    if (data.contains(item) == false)
      data.append(item);
  }
  // load data into view
  load_data_into_view(data);
}

void MainWindow::load_data_into_view(QList<QStringList> data) {
  for (int i = 0; i < data.count(); i++) {
    QStringList item_meta = data.at(i);
    if (item_meta.count() == 5) {
      QString title, copyright, fullUrl, thumbUrl, date;
      title = item_meta.at(0);
      copyright = item_meta.at(1);
      fullUrl = item_meta.at(2);
      thumbUrl = item_meta.at(3);
      date = item_meta.at(4);
      thumbUrl = thumbUrl.replace("_480", "_200");

      bool toAdd = true;
      for (int i = 0; i < ui->wallpaperList->count(); i++) {
        QWidget *listwidget =
            ui->wallpaperList->itemWidget(ui->wallpaperList->item(i));
        if (listwidget != nullptr) {
          QLineEdit *fullUrlEdit =
              listwidget->findChild<QLineEdit *>("fullUrl");
          if (fullUrlEdit->text().trimmed() == fullUrl) {
            toAdd = false;
          }
        }
        if (toAdd == false) {
          break;
        }
      }
      if (toAdd) {
        QWidget *listwidget = new QWidget(ui->wallpaperList);
        _ui_listitem.setupUi(listwidget);
        _ui_listitem.thumbUrl->setText(thumbUrl);
        _ui_listitem.thumbUrl->hide();
        _ui_listitem.fullUrl->setText(fullUrl);
        _ui_listitem.fullUrl->hide();

        listwidget->adjustSize();

        listwidget->setToolTip("<b>" + title + "</b><br>" + copyright + "<br>" +
                               date);

        QListWidgetItem *item;
        item = new QListWidgetItem(ui->wallpaperList);

        item->setSizeHint(listwidget->minimumSizeHint());

        _ui_listitem.thumbnail->setScaledContents(true);
        double ratio = 178.0 / 100.0;
        double height = listwidget->height();
        double width = ratio * height;

        _ui_listitem.thumbnail->setPixmap(
            QPixmap(":/resources/180.jpg")
                .scaled(QSize(width, height), Qt::KeepAspectRatio,
                        Qt::SmoothTransformation));
        _ui_listitem.thumbnail->setFixedSize(width, height);
        _ui_listitem.thumbnail->init(this->networkManager_, thumbUrl,
                                     "qrc:///resources/180.jpg");

        ui->wallpaperList->setItemWidget(item, listwidget);

        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
        auto *itemWidget = ui->wallpaperList->itemWidget(item);
        itemWidget->setGraphicsEffect(eff);
        QPropertyAnimation *a = new QPropertyAnimation(eff, "opacity");
        connect(a, &QPropertyAnimation::finished,
                [=]() { itemWidget->graphicsEffect()->deleteLater(); });
        a->setDuration(500);
        a->setStartValue(0);
        a->setEndValue(1);
        a->setEasingCurve(QEasingCurve::InCirc);
        a->start(QPropertyAnimation::DeleteWhenStopped);
        ui->wallpaperList->addItem(item);
        // set first item in wall_view
        if (i == 0) {
          ui->wallpaperList->setCurrentRow(0);
        }
        // keep updaing navigation buttons as we add items synchronously
        updateNavigationButtons();
      }
    }
  }
  bool containsWallpapers = data.count() > 0;
  emit wallpaperListLoaded(containsWallpapers);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_wallpaperList_itemClicked(QListWidgetItem *item) {
  QWidget *listwidget = ui->wallpaperList->itemWidget(item);
  if (listwidget != nullptr) {
    QLineEdit *fullUrl = listwidget->findChild<QLineEdit *>("fullUrl");
    this->_currentUrl = fullUrl->text();
    QLineEdit *thumbUrl = listwidget->findChild<QLineEdit *>("thumbUrl");
    currentWallInfo = listwidget->toolTip();
    LoadCover(QUrl(thumbUrl->text()), *_wall_view);
  }
  updateNavigationButtons();
}

void MainWindow::updateNavigationButtons() {
  int currentRow = ui->wallpaperList->currentRow();
  int totalItems = ui->wallpaperList->count();
  if (currentRow < totalItems - 1) {
    ui->right->setEnabled(true);
  } else {
    ui->right->setEnabled(false);
  }
  if (currentRow > 0) {
    ui->left->setEnabled(true);
  } else {
    ui->left->setEnabled(false);
  }
}

void MainWindow::on_right_clicked() {
  ui->wallpaperList->setCurrentRow(ui->wallpaperList->currentRow() + 1);
}

void MainWindow::on_left_clicked() {
  ui->wallpaperList->setCurrentRow(ui->wallpaperList->currentRow() - 1);
}

void MainWindow::on_wallpaperList_currentRowChanged(int currentRow) {
  on_wallpaperList_itemClicked(ui->wallpaperList->item(currentRow));
}

void MainWindow::on_setting_clicked() {
  if (!setting_widget->isVisible()) {
    setting_widget->move(QApplication::desktop()->geometry().center());
    check_for_startup();
    setting_widget->show();
  }
}

void MainWindow::updateAbout() {
  QString data =
      "<p align=\"center\" "
      "style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
      "margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><img "
      "src=\":/resources/binfwall_64.png\" /></p> <p align=\"center\" "
      "style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; "
      "margin-left:0px; margin-right:0px; -qt-block-indent:0; "
      "text-indent:0px;\"><br /></p> <p align=\"center\" style=\" "
      "margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
      "-qt-block-indent:0; text-indent:0px;\">Designed and Developed</p> <p "
      "align=\"center\" style=\" margin-top:0px; margin-bottom:0px; "
      "margin-left:0px; margin-right:0px; -qt-block-indent:0; "
      "text-indent:0px;\">by <span style=\" font-weight:600;\">Keshav "
      "Bhatt</span> &lt;keshavnrj@gmail.com&gt;</p> <p align=\"center\" "
      "style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; "
      "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Website: "
      "https://ktechpit.com</p> <p align=\"center\" style=\" margin-top:0px; "
      "margin-bottom:0px; margin-left:0px; margin-right:0px; "
      "-qt-block-indent:0; text-indent:0px;\">Runtime: %1</p> <p "
      "align=\"center\" style=\" margin-top:0px; margin-bottom:0px; "
      "margin-left:0px; margin-right:0px; -qt-block-indent:0; "
      "text-indent:0px;\">Version: %2</p> <p align=\"center\" "
      "style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; "
      "margin-left:0px; margin-right:0px; -qt-block-indent:0; "
      "text-indent:0px;\"></p>";
  _ui_settings.aboutTextBrowser->setText(data.arg(qVersion(), VERSIONSTR));
}

void MainWindow::on_downloaded_clicked() {
  if (!downloaded_widget->isVisible()) {
    if (downloded_loaded == false) {
      downloaded_widget->move(this->geometry().center());
      load_downloaded_wallpapers();
    }
    downloaded_widget->setMinimumWidth(215 * cols + 5 * cols + 60);
    downloaded_widget->setMinimumHeight(400);
    downloaded_widget->show();
  }
}

void MainWindow::load_downloaded_wallpapers() {
  // clear
  for (int i = 0; i < _ui_downloaded.gridLayout->columnCount(); i++) {
    GridLayoutUtil::removeColumn(_ui_downloaded.gridLayout, i, true);
  }
  QDir dir(returnPath("downloaded"));
  QStringList filter;
  filter << +"*.jpg"
         << "*.png";
  QFileInfoList files = dir.entryInfoList(filter, QDir::Files, QDir::Time);
  cols = 4;
  rows = files.count() / cols;
  _ui_downloaded.info->setText("<html><head/><body><p><span style=' "
                               "font-size:16pt; color:#9fa0a4;'>Total items: " +
                               QString::number(files.count()) +
                               "</span></p></body></html>");
  foreach (QFileInfo fileInfo, files) {
    QLabel *label = new QLabel;
    label->setAlignment(Qt::AlignCenter);
    label->setObjectName("downloaded_wall-" + fileInfo.completeBaseName());
    label->resize(215, 120);
    QPixmap picture(fileInfo.filePath());
    label->setPixmap(picture.scaled(label->size(), Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
    int row = _ui_downloaded.gridLayout->count() / cols;
    int col = _ui_downloaded.gridLayout->count() % cols;
    _ui_downloaded.gridLayout->addWidget(label, row, col);
    _ui_downloaded.gridLayout->itemAtPosition(row, col)
        ->widget()
        ->installEventFilter(this);
  }
  downloded_loaded = true;
}

void MainWindow::on_wonderwall_clicked() {
  QDesktopServices::openUrl(QUrl("https://snapcraft.io/wonderwall"));
}

void MainWindow::setWallpaperOfTheDay() {
  connect(this, &MainWindow::wallpaperSet, [=]() {
    if (this->close() == false) {
      qApp->quit();
    }
  });
  connect(this, &MainWindow::wallpaperListLoaded, [=](bool containsWallpapers) {
    if (containsWallpapers) {
      ui->wallpaperList->setCurrentRow(0);
      _ui_action.set->click();
    } else {
      // TODO retry 10 time
      if (this->close() == false) {
        qApp->quit();
      }
    }
  });
}

// run app at startup
void MainWindow::run_onstartup(bool enabled) {

  QString launcher = QApplication::applicationFilePath();
  QString launcher_name = QApplication::applicationName();
  QString data = "[Desktop Entry]\n"
                 "Type=Application\n"
                 "Exec=" +
                 launcher +
                 " --set\n"
                 "Hidden=false\n"
                 "NoDisplay=false\n"
                 "Name[en_IN]=" +
                 launcher_name +
                 "\n"
                 "Name=" +
                 launcher_name +
                 "\n"
                 "X-GNOME-Autostart-enabled=true";
  QString autostartpath = QDir::homePath() + "/.config/autostart";

  QDir dir;
  dir.mkpath(autostartpath);

  // edit file and add X-GNOME-Autostart-enabled=true
  QFile autostartfile(autostartpath + QDir::separator() + launcher_name +
                      ".desktop");

  if (enabled) {
    if (autostartfile.open(QIODevice::WriteOnly)) {
      QTextStream stream(&autostartfile);
      stream << data << endl;
    }
  } else {
    QFile(autostartpath + "/" + launcher_name + ".desktop").remove();
  }
}

// slot for startup toggled
void MainWindow::launch_on_startup_toggled(bool arg1) {
#ifdef Q_OS_LINUX
  run_onstartup(arg1);
#else
  QMessageBox::information(this, QApplication::applicationName() + " | Error",
                           "This feature is not supported on this OS");
#endif
}

// check if startup file is there .config/autostart
void MainWindow::check_for_startup() {
  QString launcher_name = QApplication::applicationName();
  QString autostartpath = QDir::homePath() + "/.config/autostart";
  QFile autostartfile(autostartpath + QDir::separator() + launcher_name +
                      ".desktop");
  _ui_settings.startUp->setChecked(QFileInfo(autostartfile).exists());
}
