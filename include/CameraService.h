#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QStringList>
#include <QSize>
#include <QtGlobal>
#include <memory>

class QCamera;
class QImageCapture;
class QMediaRecorder;
class QMediaCaptureSession;
class QVideoSink;
class QTimer;
class QWidget;
class QCameraDevice;
class CameraPreviewWidget;

struct CameraResolution {
    int width = 0;
    int height = 0;
    double minFps = 0.0;
    double maxFps = 0.0;
    QString description() const;
};

struct CameraControlDescriptor {
    QString id;
    QString label;
    QString category;
    int minimum = 0;
    int maximum = 0;
    int step = 1;
    int defaultValue = 0;
    int currentValue = 0;
    bool supportsAuto = false;
    bool supportsManual = false;
    bool autoMode = false;
};

class DirectShowCameraControls;

class CameraService final : public QObject {
    Q_OBJECT

public:
    explicit CameraService(QObject* parent = nullptr);
    ~CameraService() override;

    // 设备管理
    void refreshCameras();
    QStringList cameraNames() const;
    QWidget* previewWidget() const;

    // 相机控制
    bool openCamera(int index, QString* error = nullptr);
    void closeCamera();
    bool isOpen() const;
    int currentIndex() const;
    QString currentCameraName() const;
    QString currentCameraId() const;

    // 分辨率和帧率
    QList<CameraResolution> supportedResolutions() const;
    CameraResolution currentResolution() const;
    bool setResolution(int width, int height, QString* error = nullptr);
    bool setFrameRate(double fps, QString* error = nullptr);

    // 拍照
    bool captureImage(QString* savedPath = nullptr, QString* error = nullptr);
    void setSaveDirectory(const QString& directoryPath);
    QString saveDirectory() const;

    // 录制
    bool startRecording(QString* savedPath = nullptr, QString* error = nullptr);
    void stopRecording();
    bool isRecording() const;

    QList<CameraControlDescriptor> supportedControls() const;
    bool setCameraControl(const QString& id, int value, bool autoMode,
                          QString* error = nullptr);
    void setMonochromePreview(bool monochrome);
    bool monochromePreview() const;
    void setPreviewTransform(int mode, bool flipHorizontal, bool flipVertical);

    // 图像调整（Qt6部分支持）
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
    void resolutionChanged(int width, int height);

private:
    QString buildCapturePath() const;
    QString buildRecordPath(const QString& extension) const;
    void updateResolutions();
    void startDurationTimer();
    void stopDurationTimer();

    QStringList cameraNames_;
    QList<QCameraDevice> cameraDevices_;
    QList<CameraResolution> supportedResolutions_;
    QString saveDirectory_;
    QString currentRecordPath_;
    int currentIndex_ = -1;
    bool opened_ = false;
    bool recording_ = false;
    bool monochromePreview_ = false;
    int previewMode_ = 0;
    bool previewFlipHorizontal_ = false;
    bool previewFlipVertical_ = false;

    QCamera* camera_ = nullptr;
    std::unique_ptr<DirectShowCameraControls> directShowControls_;
    QImageCapture* imageCapture_ = nullptr;
    QMediaRecorder* recorder_ = nullptr;
    QMediaCaptureSession* captureSession_ = nullptr;
    QVideoSink* videoSink_ = nullptr;
    CameraPreviewWidget* videoWidget_ = nullptr;
    QTimer* durationTimer_ = nullptr;
    QElapsedTimer recordingElapsed_;

};
