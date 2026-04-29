#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QStringList>
#include <memory>

class QCamera;
class QCameraImageCapture;
class QMediaRecorder;
class QCameraViewfinder;
class QTimer;
class QWidget;
#ifdef RTC_HAS_OPENCV
namespace cv {
class VideoCapture;
class VideoWriter;
}
#endif

class CameraService final : public QObject {
    Q_OBJECT

public:
    explicit CameraService(QObject* parent = nullptr);
    ~CameraService() override;

    void refreshCameras();
    QStringList cameraNames() const;

    QWidget* previewWidget() const;

    bool openCamera(int index, QString* error = nullptr);
    void closeCamera();
    bool isOpen() const;
    int currentIndex() const;

    bool captureImage(QString* savedPath = nullptr, QString* error = nullptr);
    void setSaveDirectory(const QString& directoryPath);
    QString saveDirectory() const;

    bool startRecording(QString* savedPath = nullptr, QString* error = nullptr);
    void stopRecording();
    bool isRecording() const;

    bool applyImageSettings(double brightness, double contrast, double saturation,
                            double exposureCompensation, bool monochrome,
                            QString* error = nullptr);
    bool readImageSettings(double& brightness, double& contrast, double& saturation,
                           double& exposureCompensation, bool& monochrome,
                           QString* error = nullptr) const;

signals:
    void cameraListChanged(const QStringList& names);
    void cameraStateChanged(bool opened, const QString& name);
    void recordingStateChanged(bool recording, const QString& filePath);
    void recordingDurationChanged(qint64 msec);
    void cameraLog(const QString& message);

private:
    QString buildCapturePath() const;

    QStringList cameraNames_;
    QString saveDirectory_;
    QString currentRecordPath_;
    int currentIndex_ = -1;
    bool opened_ = false;
    bool recording_ = false;
    bool usingOpenCvRecording_ = false;
    bool openCvPausedQtCamera_ = false;

    QCamera* camera_ = nullptr;
    QCameraImageCapture* imageCapture_ = nullptr;
    QMediaRecorder* recorder_ = nullptr;
    QCameraViewfinder* preview_ = nullptr;
    QTimer* openCvRecordTimer_ = nullptr;
    QElapsedTimer openCvElapsed_;
#ifdef RTC_HAS_OPENCV
    std::unique_ptr<cv::VideoCapture> openCvCapture_;
    std::unique_ptr<cv::VideoWriter> openCvWriter_;
    int preferredOpenCvBackend_ = -1;
    bool openCvPrewarmDone_ = false;
#endif
};
