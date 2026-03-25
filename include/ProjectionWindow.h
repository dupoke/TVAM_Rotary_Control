#pragma once

#include <QPixmap>
#include <QWidget>

class QLabel;

class ProjectionWindow final : public QWidget {
    Q_OBJECT

public:
    explicit ProjectionWindow(QWidget* parent = nullptr);

    void showForProjection();
    void hideProjection();
    void displayImage(const QString& filePath);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void refreshScaledPixmap();

    QLabel* imageLabel_ = nullptr;
    QPixmap currentPixmap_;
};
