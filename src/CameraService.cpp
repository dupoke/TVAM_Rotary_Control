#include "CameraService.h"

#include <QCamera>
#include <QCameraExposure>
#include <QCameraImageCapture>
#include <QCameraImageProcessing>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QMediaRecorder>
#include <QTimer>
#include <QUrl>
#include <algorithm>
#include <limits>
#include <vector>
#ifdef RTC_HAS_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#endif

namespace {
QString ensureDefaultSaveDirectory() {
    const QString dirPath = QDir::current().filePath(QStringLiteral("captures"));
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    return QDir(dirPath).absolutePath();
}

#ifdef RTC_HAS_OPENCV
QString openCvBackendName(int backend) {
    switch (backend) {
        case cv::CAP_DSHOW:
            return QStringLiteral("DSHOW");
        case cv::CAP_MSMF:
            return QStringLiteral("MSMF");
        case cv::CAP_ANY:
            return QStringLiteral("ANY");
        default:
            return QStringLiteral("ID=%1").arg(backend);
    }
}
#endif
}  // namespace

CameraService::CameraService(QObject* parent)
    : QObject(parent),
      saveDirectory_(ensureDefaultSaveDirectory()),
      preview_(new QCameraViewfinder()) {
    preview_->setObjectName(QStringLiteral("cameraPreview"));
    preview_->setMinimumSize(480, 270);
    preview_->setStyleSheet(QStringLiteral("QWidget{background:#1f1f1f;border:1px solid #555;}"));
    openCvRecordTimer_ = new QTimer(this);
    openCvRecordTimer_->setTimerType(Qt::PreciseTimer);
#ifdef RTC_HAS_OPENCV
    connect(openCvRecordTimer_, &QTimer::timeout, this, [this]() {
        if (!usingOpenCvRecording_ || openCvCapture_ == nullptr || openCvWriter_ == nullptr) {
            return;
        }
        cv::Mat frame;
        if (!openCvCapture_->read(frame) || frame.empty()) {
            return;
        }
        openCvWriter_->write(frame);
        emit recordingDurationChanged(openCvElapsed_.elapsed());
    });
#endif
}

CameraService::~CameraService() {
    closeCamera();
    delete preview_;
    preview_ = nullptr;
}

void CameraService::refreshCameras() {
    cameraNames_.clear();
    const auto cameras = QCameraInfo::availableCameras();
    for (const QCameraInfo& info : cameras) {
        cameraNames_.push_back(info.description());
    }
    emit cameraListChanged(cameraNames_);
    emit cameraLog(QStringLiteral("检测到相机数量: %1").arg(cameraNames_.size()));
}

QStringList CameraService::cameraNames() const {
    return cameraNames_;
}

QWidget* CameraService::previewWidget() const {
    return preview_;
}

bool CameraService::openCamera(int index, QString* error) {
    const auto cameras = QCameraInfo::availableCameras();
    if (index < 0 || index >= cameras.size()) {
        if (error != nullptr) {
            *error = QStringLiteral("相机索引无效。");
        }
        return false;
    }

    closeCamera();

#ifdef RTC_HAS_OPENCV
    preferredOpenCvBackend_ = -1;
    openCvPrewarmDone_ = false;
    {
        const int backends[] = {cv::CAP_MSMF, cv::CAP_DSHOW, cv::CAP_ANY};
        qint64 bestCostMs = std::numeric_limits<qint64>::max();
        for (const int backend : backends) {
            QElapsedTimer warmTimer;
            warmTimer.start();
            cv::VideoCapture warmCap;
            if (!warmCap.open(index, backend)) {
                continue;
            }
            cv::Mat warmFrame;
            bool ok = false;
            while (warmTimer.elapsed() < 220) {
                if (warmCap.read(warmFrame) && !warmFrame.empty()) {
                    ok = true;
                    break;
                }
                QCoreApplication::processEvents(QEventLoop::AllEvents, 8);
            }
            warmCap.release();
            if (!ok) {
                continue;
            }
            const qint64 costMs = warmTimer.elapsed();
            if (costMs < bestCostMs) {
                bestCostMs = costMs;
                preferredOpenCvBackend_ = backend;
                openCvPrewarmDone_ = true;
            }
        }
        if (openCvPrewarmDone_) {
            emit cameraLog(
                QStringLiteral("录像预热完成: 后端=%1, 预热耗时=%2ms")
                    .arg(openCvBackendName(preferredOpenCvBackend_))
                    .arg(bestCostMs));
        }
        if (!openCvPrewarmDone_) {
            emit cameraLog(QStringLiteral("录像预热未完成: 将在启动录像时即时初始化。"));
        }
    }
#endif

    camera_ = new QCamera(cameras.at(index), this);
    camera_->setCaptureMode(QCamera::CaptureStillImage);
    camera_->setViewfinder(preview_);

    imageCapture_ = new QCameraImageCapture(camera_, this);
    imageCapture_->setCaptureDestination(QCameraImageCapture::CaptureToFile);

    recorder_ = new QMediaRecorder(camera_, this);
    emit cameraLog(QStringLiteral("录像能力: 容器=%1, 视频编码=%2, 音频编码=%3")
                       .arg(recorder_->supportedContainers().join(QStringLiteral(",")))
                       .arg(recorder_->supportedVideoCodecs().join(QStringLiteral(",")))
                       .arg(recorder_->supportedAudioCodecs().join(QStringLiteral(","))));
#ifdef RTC_HAS_OPENCV
    emit cameraLog(QStringLiteral("OpenCV录像模式: 已启用（默认）。"));
#else
    emit cameraLog(QStringLiteral("OpenCV录像模式: 未启用（将使用Qt录像）。"));
#endif

    connect(camera_, static_cast<void (QCamera::*)(QCamera::Error)>(&QCamera::error), this,
            [this](QCamera::Error err) {
                if (err != QCamera::NoError && camera_ != nullptr) {
                    emit cameraLog(QStringLiteral("相机错误: %1").arg(camera_->errorString()));
                }
            });
    connect(imageCapture_,
            static_cast<void (QCameraImageCapture::*)(int, QCameraImageCapture::Error, const QString&)>(
                &QCameraImageCapture::error),
            this,
            [this](int, QCameraImageCapture::Error, const QString& errorString) {
                emit cameraLog(QStringLiteral("拍照失败: %1").arg(errorString));
            });
    connect(imageCapture_, &QCameraImageCapture::imageSaved, this,
            [this](int, const QString& filePath) {
                emit cameraLog(QStringLiteral("拍照保存成功: %1").arg(QDir::toNativeSeparators(filePath)));
            });

    connect(recorder_, static_cast<void (QMediaRecorder::*)(QMediaRecorder::Error)>(&QMediaRecorder::error),
            this,
            [this](QMediaRecorder::Error err) {
                if (err != QMediaRecorder::NoError && recorder_ != nullptr) {
                    emit cameraLog(QStringLiteral("录像错误: %1").arg(recorder_->errorString()));
                }
            });
    connect(recorder_, &QMediaRecorder::statusChanged, this,
            [this](QMediaRecorder::Status status) {
                if (status == QMediaRecorder::FinalizingStatus) {
                    emit cameraLog(QStringLiteral("录像文件写入中..."));
                }
                if (status == QMediaRecorder::UnavailableStatus) {
                    emit cameraLog(QStringLiteral("录像状态异常: 录制器不可用。"));
                }
            });
    connect(recorder_, &QMediaRecorder::stateChanged, this,
            [this](QMediaRecorder::State state) {
                const bool newRecording = (state == QMediaRecorder::RecordingState);
                if (recording_ != newRecording) {
                    recording_ = newRecording;
                    emit recordingStateChanged(recording_, currentRecordPath_);
                    emit cameraLog(recording_ ? QStringLiteral("录像开始。") : QStringLiteral("录像停止。"));
                }
                if (!recording_) {
                    emit recordingDurationChanged(0);
                }
            });
    connect(recorder_, &QMediaRecorder::durationChanged, this,
            [this](qint64 duration) { emit recordingDurationChanged(duration); });

    camera_->start();
    if (camera_->state() != QCamera::ActiveState) {
        if (error != nullptr) {
            *error = camera_->errorString().isEmpty() ? QStringLiteral("相机启动失败。")
                                                      : camera_->errorString();
        }
        closeCamera();
        return false;
    }

    opened_ = true;
    currentIndex_ = index;
    emit cameraStateChanged(true, cameras.at(index).description());
    emit recordingDurationChanged(0);
    emit cameraLog(QStringLiteral("相机已打开: %1").arg(cameras.at(index).description()));
    return true;
}

void CameraService::closeCamera() {
    const bool wasOpen = opened_;
    stopRecording();

    if (recorder_ != nullptr) {
        if (recorder_->state() == QMediaRecorder::RecordingState) {
            recorder_->stop();
        }
        recorder_->deleteLater();
        recorder_ = nullptr;
    }

    if (camera_ != nullptr) {
        camera_->stop();
    }
    if (imageCapture_ != nullptr) {
        imageCapture_->deleteLater();
        imageCapture_ = nullptr;
    }
    if (camera_ != nullptr) {
        camera_->deleteLater();
        camera_ = nullptr;
    }

    opened_ = false;
    currentIndex_ = -1;
    recording_ = false;
    usingOpenCvRecording_ = false;
    currentRecordPath_.clear();
#ifdef RTC_HAS_OPENCV
    if (openCvWriter_ != nullptr) {
        openCvWriter_->release();
        openCvWriter_.reset();
    }
    if (openCvCapture_ != nullptr) {
        openCvCapture_->release();
        openCvCapture_.reset();
    }
    preferredOpenCvBackend_ = -1;
    openCvPrewarmDone_ = false;
#endif

    if (wasOpen) {
        emit cameraStateChanged(false, QStringLiteral("N/A"));
        emit cameraLog(QStringLiteral("相机已关闭。"));
    }
}

bool CameraService::isOpen() const {
    return opened_;
}

int CameraService::currentIndex() const {
    return currentIndex_;
}

bool CameraService::captureImage(QString* savedPath, QString* error) {
    if (!opened_ || imageCapture_ == nullptr || camera_ == nullptr) {
        if (error != nullptr) {
            *error = QStringLiteral("相机未打开，无法拍照。");
        }
        return false;
    }

    if (camera_->captureMode() != QCamera::CaptureStillImage) {
        camera_->setCaptureMode(QCamera::CaptureStillImage);
    }

    if (!imageCapture_->isReadyForCapture()) {
        if (error != nullptr) {
            *error = QStringLiteral("相机当前不可拍照，请稍后重试。");
        }
        return false;
    }

    const QString path = buildCapturePath();
    imageCapture_->capture(path);
    if (savedPath != nullptr) {
        *savedPath = path;
    }
    return true;
}

void CameraService::setSaveDirectory(const QString& directoryPath) {
    QString path = directoryPath.trimmed();
    if (path.isEmpty()) {
        path = ensureDefaultSaveDirectory();
    }

    QDir dir(path);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    saveDirectory_ = QFileInfo(dir.absolutePath()).absoluteFilePath();
    emit cameraLog(QStringLiteral("相机保存目录: %1").arg(QDir::toNativeSeparators(saveDirectory_)));
}

QString CameraService::saveDirectory() const {
    return saveDirectory_;
}

bool CameraService::startRecording(QString* savedPath, QString* error) {
    if (!opened_ || camera_ == nullptr || recorder_ == nullptr) {
        if (error != nullptr) {
            *error = QStringLiteral("相机未打开，无法录像。");
        }
        return false;
    }
    if (recording_) {
        if (savedPath != nullptr) {
            *savedPath = currentRecordPath_;
        }
        return true;
    }
    usingOpenCvRecording_ = false;
    openCvPausedQtCamera_ = false;

#ifdef RTC_HAS_OPENCV
    // Fixed exclusive path: always pause Qt preview and use OpenCV capture directly.
    if (recorder_ != nullptr && recorder_->state() == QMediaRecorder::RecordingState) {
        recorder_->stop();
    }

    auto tryReadFirstFrame = [this](cv::Mat& outFrame, int timeoutMs) -> bool {
        if (openCvCapture_ == nullptr) {
            return false;
        }
        QElapsedTimer waitTimer;
        waitTimer.start();
        while (waitTimer.elapsed() < timeoutMs) {
            if (openCvCapture_->read(outFrame) && !outFrame.empty()) {
                return true;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 8);
        }
        return false;
    };

    if (camera_ != nullptr) {
        camera_->stop();
        openCvPausedQtCamera_ = true;
    }

    if (openCvCapture_ != nullptr) {
        openCvCapture_->release();
        openCvCapture_.reset();
    }
    openCvCapture_ = std::make_unique<cv::VideoCapture>();

    auto tryOpenCaptureExclusive = [this]() -> bool {
        std::vector<int> backends;
        if (preferredOpenCvBackend_ >= 0) {
            backends.push_back(preferredOpenCvBackend_);
        }
#ifdef Q_OS_WIN
        const int defaults[] = {cv::CAP_MSMF, cv::CAP_DSHOW, cv::CAP_ANY};
#else
        const int defaults[] = {cv::CAP_ANY};
#endif
        for (const int backend : defaults) {
            if (std::find(backends.begin(), backends.end(), backend) == backends.end()) {
                backends.push_back(backend);
            }
        }
        for (const int backend : backends) {
            if (openCvCapture_->open(currentIndex_, backend)) {
                preferredOpenCvBackend_ = backend;
                return true;
            }
        }
        return false;
    };

    const bool openOk = tryOpenCaptureExclusive();
    cv::Mat firstFrame;
    const bool firstFrameOk = openOk ? tryReadFirstFrame(firstFrame, 700) : false;
    if (!openOk || !firstFrameOk) {
        if (error != nullptr) {
            *error = QStringLiteral("OpenCV录像初始化失败：无法稳定获取视频帧。");
        }
        if (openCvCapture_ != nullptr) {
            openCvCapture_->release();
            openCvCapture_.reset();
        }
        if (camera_ != nullptr && openCvPausedQtCamera_) {
            camera_->start();
        }
        openCvPausedQtCamera_ = false;
        currentRecordPath_.clear();
        return false;
    }

    const int width = firstFrame.cols;
    const int height = firstFrame.rows;
    double fps = openCvCapture_->get(cv::CAP_PROP_FPS);
    if (fps < 1.0 || fps > 120.0) {
        fps = 30.0;
    }

    QDir cvDir(saveDirectory_.isEmpty() ? ensureDefaultSaveDirectory() : saveDirectory_);
    if (!cvDir.exists()) {
        cvDir.mkpath(QStringLiteral("."));
    }
    const QString cvTimestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"));
    const QString aviPath = cvDir.filePath(QStringLiteral("video_%1.avi").arg(cvTimestamp));
    const QString mp4Path = cvDir.filePath(QStringLiteral("video_%1.mp4").arg(cvTimestamp));

    openCvWriter_ = std::make_unique<cv::VideoWriter>();
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    currentRecordPath_ = aviPath;
    if (!openCvWriter_->open(currentRecordPath_.toStdString(), fourcc, fps, cv::Size(width, height))) {
        fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
        currentRecordPath_ = mp4Path;
        if (!openCvWriter_->open(currentRecordPath_.toStdString(), fourcc, fps,
                                 cv::Size(width, height))) {
            if (error != nullptr) {
                *error = QStringLiteral("OpenCV录像写入器启动失败。");
            }
            openCvWriter_.reset();
            openCvCapture_->release();
            openCvCapture_.reset();
            if (camera_ != nullptr && openCvPausedQtCamera_) {
                camera_->start();
            }
            openCvPausedQtCamera_ = false;
            currentRecordPath_.clear();
            return false;
        }
    }

    const int intervalMs = qBound(10, static_cast<int>(qRound(1000.0 / fps)), 60);
    openCvElapsed_.start();
    usingOpenCvRecording_ = true;
    recording_ = true;
    openCvWriter_->write(firstFrame);
    if (savedPath != nullptr) {
        *savedPath = currentRecordPath_;
    }
    emit recordingStateChanged(true, currentRecordPath_);
    emit recordingDurationChanged(0);
    emit cameraLog(QStringLiteral("录像中: %1").arg(QDir::toNativeSeparators(currentRecordPath_)));
    emit cameraLog(QStringLiteral("OpenCV录像启动成功: %1")
                       .arg(QDir::toNativeSeparators(currentRecordPath_)));
    openCvRecordTimer_->start(intervalMs);
    emit cameraLog(QStringLiteral("录像启动后端: %1")
                       .arg(openCvBackendName(preferredOpenCvBackend_)));
    return true;
#else
    const bool videoModeSupported = camera_->isCaptureModeSupported(QCamera::CaptureVideo);
    if (!videoModeSupported) {
        emit cameraLog(QStringLiteral("相机报告不支持 CaptureVideo，尝试以兼容方式启动录像。"));
    }
    if (camera_->captureMode() != QCamera::CaptureVideo) {
        camera_->setCaptureMode(QCamera::CaptureVideo);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 30);
    }

    QString containerFormat;
    QString fileExt = QStringLiteral("mp4");
    const QStringList containers = recorder_->supportedContainers();
    if (containers.contains(QStringLiteral("video/mp4"))) {
        containerFormat = QStringLiteral("video/mp4");
        fileExt = QStringLiteral("mp4");
    } else if (containers.contains(QStringLiteral("video/x-msvideo"))) {
        containerFormat = QStringLiteral("video/x-msvideo");
        fileExt = QStringLiteral("avi");
    } else if (containers.contains(QStringLiteral("video/x-ms-wmv"))) {
        containerFormat = QStringLiteral("video/x-ms-wmv");
        fileExt = QStringLiteral("wmv");
    } else if (!containers.isEmpty()) {
        containerFormat = containers.first();
        fileExt = QStringLiteral("mp4");
    }
    if (!containerFormat.isEmpty()) {
        recorder_->setContainerFormat(containerFormat);
    }

    QDir dir(saveDirectory_.isEmpty() ? ensureDefaultSaveDirectory() : saveDirectory_);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    const QString timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"));
    currentRecordPath_ = dir.filePath(QStringLiteral("video_%1.%2").arg(timestamp, fileExt));
    recorder_->setOutputLocation(QUrl::fromLocalFile(currentRecordPath_));
    emit recordingDurationChanged(0);
    emit cameraLog(QStringLiteral("录像启动请求: 文件=%1，容器=%2")
                       .arg(QDir::toNativeSeparators(currentRecordPath_))
                       .arg(containerFormat.isEmpty() ? QStringLiteral("默认") : containerFormat));
    auto waitForRecording = [this]() {
        QElapsedTimer waitTimer;
        waitTimer.start();
        while (waitTimer.elapsed() < 800 &&
               recorder_->state() != QMediaRecorder::RecordingState &&
               recorder_->error() == QMediaRecorder::NoError) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        }
        return recorder_->state() == QMediaRecorder::RecordingState;
    };

    recorder_->record();

    if (savedPath != nullptr) {
        *savedPath = currentRecordPath_;
    }

    if (!waitForRecording()) {
        // Fallback: some drivers report unsupported video mode but can record from still mode.
        if (camera_->captureMode() != QCamera::CaptureStillImage) {
            camera_->setCaptureMode(QCamera::CaptureStillImage);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 30);
        }
        recorder_->record();
    }

    if (waitForRecording()) {
        return true;
    }

    if (!waitForRecording()) {
        if (error != nullptr) {
            const QString fallback =
                QStringLiteral("录像未进入录制状态，请检查容器/编码器支持。");
            *error = recorder_->errorString().isEmpty() ? fallback : recorder_->errorString();
        }
        recorder_->stop();
        currentRecordPath_.clear();
        return false;
    }
    return true;
#endif
}

void CameraService::stopRecording() {
    if (usingOpenCvRecording_) {
#ifdef RTC_HAS_OPENCV
        usingOpenCvRecording_ = false;
        if (openCvRecordTimer_ != nullptr) {
            openCvRecordTimer_->stop();
        }
        if (openCvWriter_ != nullptr) {
            openCvWriter_->release();
            openCvWriter_.reset();
        }
        if (openCvCapture_ != nullptr) {
            openCvCapture_->release();
            openCvCapture_.reset();
        }
#endif
        if (camera_ != nullptr && openCvPausedQtCamera_) {
            camera_->start();
        }
        openCvPausedQtCamera_ = false;
        if (recording_) {
            recording_ = false;
            emit recordingStateChanged(false, currentRecordPath_);
            emit recordingDurationChanged(0);
            emit cameraLog(QStringLiteral("OpenCV录像已停止。"));
        }
        return;
    }

    if (recorder_ == nullptr) {
        return;
    }
    if (recorder_->state() == QMediaRecorder::RecordingState) {
        recorder_->stop();
    }
    if (camera_ != nullptr && camera_->captureMode() != QCamera::CaptureStillImage) {
        camera_->setCaptureMode(QCamera::CaptureStillImage);
    }
}

bool CameraService::isRecording() const {
    return recording_;
}

bool CameraService::applyImageSettings(double brightness, double contrast, double saturation,
                                       double exposureCompensation, bool monochrome,
                                       QString* error) {
    if (!opened_ || camera_ == nullptr) {
        if (error != nullptr) {
            *error = QStringLiteral("相机未打开，无法设置参数。");
        }
        return false;
    }

    QCameraImageProcessing* image = camera_->imageProcessing();
    if (image == nullptr || !image->isAvailable()) {
        if (error != nullptr) {
            *error = QStringLiteral("当前相机不支持图像参数调节。");
        }
        return false;
    }

    const double effectiveBrightness = qBound(-1.0, brightness + exposureCompensation * 0.2, 1.0);
    QCameraExposure* exposure = camera_->exposure();
    if (exposure != nullptr) {
        exposure->setExposureCompensation(exposureCompensation);
    }

    image->setBrightness(effectiveBrightness);
    image->setContrast(contrast);
    image->setSaturation(monochrome ? -1.0 : saturation);
    image->setColorFilter(monochrome ? QCameraImageProcessing::ColorFilterGrayscale
                                     : QCameraImageProcessing::ColorFilterNone);
    return true;
}

bool CameraService::readImageSettings(double& brightness, double& contrast, double& saturation,
                                      double& exposureCompensation, bool& monochrome,
                                      QString* error) const {
    if (!opened_ || camera_ == nullptr) {
        if (error != nullptr) {
            *error = QStringLiteral("相机未打开，无法读取参数。");
        }
        return false;
    }

    QCameraImageProcessing* image = camera_->imageProcessing();
    if (image == nullptr || !image->isAvailable()) {
        if (error != nullptr) {
            *error = QStringLiteral("当前相机不支持图像参数读取。");
        }
        return false;
    }

    brightness = image->brightness();
    contrast = image->contrast();
    saturation = image->saturation();
    monochrome = false;

    QCameraExposure* exposure = camera_->exposure();
    exposureCompensation = exposure != nullptr ? exposure->exposureCompensation() : 0.0;
    return true;
}

QString CameraService::buildCapturePath() const {
    QDir dir(saveDirectory_.isEmpty() ? ensureDefaultSaveDirectory() : saveDirectory_);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    const QString timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"));
    return dir.filePath(QStringLiteral("cam_%1.jpg").arg(timestamp));
}
