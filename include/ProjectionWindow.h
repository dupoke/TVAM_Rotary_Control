#pragma once

#include <QPixmap>
#include <QWidget>

class QLabel;
class QSize;

class ProjectionWindow final : public QWidget {
    Q_OBJECT

public:
    enum class ContentMode {
        Empty,
        Black,
        Image,
        SiemensStar,
        ThreeLineAlignment,
    };

    explicit ProjectionWindow(QWidget* parent = nullptr);

    void showForProjection();
    void hideProjection();
    void showBlackFrame();
    void displayImage(const QString& filePath);
    void showSiemensStarAlignment();
    void showThreeLineAlignment();
    void clearAlignment();
    bool isAlignmentActive() const;
    bool handleAlignmentKey(int key);
    ContentMode contentMode() const;
    QPixmap currentPixmap() const;
    QString currentStatusText() const;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QSize renderTargetSize() const;
    void renderCurrentContent();
    void refreshScaledPixmap();

    ContentMode contentMode_ = ContentMode::Empty;
    QLabel* imageLabel_ = nullptr;
    QPixmap currentPixmap_;
    QString currentImagePath_;
    int siemensSlices_ = 5;
    double siemensRotationDeg_ = 0.0;
    int lineHalfThickness_ = 5;
    int lineHalfSeparation_ = 200;
};
