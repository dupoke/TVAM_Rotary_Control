#include "ProjectionWindow.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>

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
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.size() > 1 && screens[1] != nullptr) {
        setGeometry(screens[1]->geometry());
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

void ProjectionWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    refreshScaledPixmap();
}

void ProjectionWindow::refreshScaledPixmap() {
    if (currentPixmap_.isNull()) {
        return;
    }
    imageLabel_->setText(QString());
    imageLabel_->setPixmap(currentPixmap_.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
