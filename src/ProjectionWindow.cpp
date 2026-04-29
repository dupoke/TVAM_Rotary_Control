#include "ProjectionWindow.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QSize>
#include <QVBoxLayout>
#include <QWindow>
#include <QtMath>

namespace {

QScreen* pickProjectionScreen() {
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        return nullptr;
    }

    QScreen* primary = QGuiApplication::primaryScreen();
    for (QScreen* screen : screens) {
        if (screen != nullptr && screen != primary) {
            return screen;
        }
    }

    return screens.first();
}

QPixmap buildSiemensStarPixmap(const QSize& size, int slices, double rotationDeg) {
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::black);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);

    const QRectF bounds(0.0, 0.0, size.width(), size.height());
    const double radius = qMin(size.width(), size.height()) * 0.475;
    const QRectF circleRect((size.width() * 0.5) - radius, (size.height() * 0.5) - radius,
                            radius * 2.0, radius * 2.0);
    const double sectorSpanDeg = 180.0 / qMax(1, slices);
    for (int i = 0; i < slices; ++i) {
        const double startDeg = rotationDeg + i * sectorSpanDeg * 2.0;
        painter.drawPie(circleRect, qRound(startDeg * 16.0), qRound(sectorSpanDeg * 16.0));
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::white, qMax(1, qRound(radius * 0.01))));
    painter.drawEllipse(circleRect);
    Q_UNUSED(bounds);
    return QPixmap::fromImage(image);
}

QPixmap buildThreeLinePixmap(const QSize& size, int lineThickness, int halfSeparation) {
    QImage image(size, QImage::Format_RGB32);
    image.fill(Qt::black);

    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);

    const int centerX = size.width() / 2;
    const int lineWidth = qMax(1, lineThickness);
    const int clampedSeparation =
        qBound(1, halfSeparation, qMax(1, (size.width() / 2) - lineWidth - 2));

    auto fillLine = [&](int centerLineX) {
        const int x = centerLineX - (lineWidth / 2);
        painter.fillRect(x, 0, lineWidth, size.height(), Qt::white);
    };

    fillLine(centerX);
    fillLine(centerX - clampedSeparation);
    fillLine(centerX + clampedSeparation);
    return QPixmap::fromImage(image);
}

}  // namespace

ProjectionWindow::ProjectionWindow(QWidget* parent)
    : QWidget(parent) {
    setWindowTitle(QStringLiteral("同步投影窗口"));
    setStyleSheet(QStringLiteral("background-color: black;"));
    setMinimumSize(640, 360);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    imageLabel_ = new QLabel(this);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setStyleSheet(QStringLiteral("color: white; font-size: 16px;"));
    imageLabel_->setText(QStringLiteral("等待同步开始..."));
    layout->addWidget(imageLabel_);
}

void ProjectionWindow::showForProjection() {
    QScreen* targetScreen = pickProjectionScreen();
    const bool hasSecondaryScreen =
        targetScreen != nullptr && targetScreen != QGuiApplication::primaryScreen();

    if (hasSecondaryScreen) {
        createWinId();
        if (windowHandle() != nullptr) {
            windowHandle()->setScreen(targetScreen);
        }
        setGeometry(targetScreen->geometry());
        showFullScreen();
    } else {
        resize(960, 540);
        show();
    }
    raise();
    activateWindow();
}

void ProjectionWindow::hideProjection() {
    hide();
}

void ProjectionWindow::displayImage(const QString& filePath) {
    contentMode_ = ContentMode::Image;
    currentImagePath_ = filePath;
    const QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        currentPixmap_ = QPixmap();
        imageLabel_->setPixmap(QPixmap());
        imageLabel_->setText(QStringLiteral("图片加载失败: %1").arg(QFileInfo(filePath).fileName()));
        return;
    }

    currentPixmap_ = pixmap;
    refreshScaledPixmap();
}

void ProjectionWindow::showSiemensStarAlignment() {
    contentMode_ = ContentMode::SiemensStar;
    renderCurrentContent();
}

void ProjectionWindow::showThreeLineAlignment() {
    contentMode_ = ContentMode::ThreeLineAlignment;
    renderCurrentContent();
}

void ProjectionWindow::clearAlignment() {
    if (!isAlignmentActive()) {
        return;
    }

    contentMode_ = ContentMode::Empty;
    currentPixmap_ = QPixmap();
    imageLabel_->setPixmap(QPixmap());
    imageLabel_->setText(QStringLiteral("等待同步开始..."));
}

bool ProjectionWindow::isAlignmentActive() const {
    return contentMode_ == ContentMode::SiemensStar ||
           contentMode_ == ContentMode::ThreeLineAlignment;
}

bool ProjectionWindow::handleAlignmentKey(int key) {
    if (!isAlignmentActive()) {
        return false;
    }

    switch (contentMode_) {
    case ContentMode::SiemensStar:
        if (key == Qt::Key_Up) {
            siemensSlices_ = qMin(200, siemensSlices_ + 1);
        } else if (key == Qt::Key_Down) {
            siemensSlices_ = qMax(4, siemensSlices_ - 1);
        } else if (key == Qt::Key_Left) {
            siemensRotationDeg_ -= 2.0;
        } else if (key == Qt::Key_Right) {
            siemensRotationDeg_ += 2.0;
        } else {
            return false;
        }
        break;
    case ContentMode::ThreeLineAlignment:
        if (key == Qt::Key_Up) {
            lineHalfThickness_ = qMin(400, lineHalfThickness_ + 1);
        } else if (key == Qt::Key_Down) {
            lineHalfThickness_ = qMax(1, lineHalfThickness_ - 1);
        } else if (key == Qt::Key_Left) {
            lineHalfSeparation_ += 2;
        } else if (key == Qt::Key_Right) {
            lineHalfSeparation_ = qMax(1, lineHalfSeparation_ - 2);
        } else {
            return false;
        }
        break;
    default:
        return false;
    }

    while (siemensRotationDeg_ < 0.0) {
        siemensRotationDeg_ += 360.0;
    }
    while (siemensRotationDeg_ >= 360.0) {
        siemensRotationDeg_ -= 360.0;
    }

    renderCurrentContent();
    return true;
}

ProjectionWindow::ContentMode ProjectionWindow::contentMode() const {
    return contentMode_;
}

QPixmap ProjectionWindow::currentPixmap() const {
    return currentPixmap_;
}

QString ProjectionWindow::currentStatusText() const {
    switch (contentMode_) {
    case ContentMode::SiemensStar:
        return QStringLiteral("当前投影: 西门子星对准  个数=%1  旋转=%2°")
            .arg(siemensSlices_)
            .arg(siemensRotationDeg_, 0, 'f', 1);
    case ContentMode::ThreeLineAlignment:
        return QStringLiteral("当前投影: 三线条对准  线宽=%1px  半线距=%2px")
            .arg(lineHalfThickness_)
            .arg(lineHalfSeparation_);
    case ContentMode::Image:
        return QStringLiteral("当前投影: 图片模式");
    case ContentMode::Empty:
    default:
        return QStringLiteral("当前投影: N/A");
    }
}

void ProjectionWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (isAlignmentActive()) {
        renderCurrentContent();
    } else {
        refreshScaledPixmap();
    }
}

QSize ProjectionWindow::renderTargetSize() const {
    const QSize currentSize = size();
    if (currentSize.width() > 0 && currentSize.height() > 0) {
        return currentSize;
    }

    if (QScreen* screen = pickProjectionScreen(); screen != nullptr) {
        return screen->geometry().size();
    }
    return QSize(1920, 1080);
}

void ProjectionWindow::renderCurrentContent() {
    const QSize targetSize = renderTargetSize();
    if (targetSize.width() <= 0 || targetSize.height() <= 0) {
        return;
    }

    switch (contentMode_) {
    case ContentMode::SiemensStar:
        currentPixmap_ = buildSiemensStarPixmap(targetSize, siemensSlices_, siemensRotationDeg_);
        break;
    case ContentMode::ThreeLineAlignment:
        currentPixmap_ = buildThreeLinePixmap(targetSize, lineHalfThickness_, lineHalfSeparation_);
        break;
    case ContentMode::Image:
        if (!currentImagePath_.isEmpty()) {
            currentPixmap_ = QPixmap(currentImagePath_);
        }
        break;
    case ContentMode::Empty:
    default:
        currentPixmap_ = QPixmap();
        break;
    }

    if (currentPixmap_.isNull()) {
        imageLabel_->setPixmap(QPixmap());
        imageLabel_->setText(QStringLiteral("图案生成失败。"));
        return;
    }

    refreshScaledPixmap();
}

void ProjectionWindow::refreshScaledPixmap() {
    if (currentPixmap_.isNull()) {
        imageLabel_->setPixmap(QPixmap());
        return;
    }
    imageLabel_->setText(QString());
    imageLabel_->setPixmap(currentPixmap_.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
