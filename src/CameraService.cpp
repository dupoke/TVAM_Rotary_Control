#include "CameraService.h"

#include <QCamera>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QMediaRecorder>
#include <QMediaFormat>
#include <QVideoSink>
#include <QWidget>
#include <QPaintEvent>
#include <QImage>
#include <QSizePolicy>
#include <QCameraDevice>
#include <QCameraFormat>
#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>
#include <QPainter>
#include <algorithm>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dshow.h>
#include <vidcap.h>
#include <oleauto.h>
#endif

namespace {
QImage transformCameraImage(QImage image, int mode, bool flipHorizontal, bool flipVertical) {
    if (image.isNull()) {
        return image;
    }
    if (flipHorizontal || flipVertical) {
        image = image.mirrored(flipHorizontal, flipVertical);
    }
    if (mode == 1) {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    } else if (mode == 2) {
        image = image.convertToFormat(QImage::Format_RGB32);
        image.invertPixels(QImage::InvertRgb);
    }
    return image;
}

bool chooseSupportedVideoFormat(QMediaFormat& selectedFormat, QString& fileExtension) {
    struct FormatOption {
        QMediaFormat::FileFormat container;
        QMediaFormat::VideoCodec codec;
        const char* extension;
    };
    static const FormatOption options[] = {
        {QMediaFormat::MPEG4, QMediaFormat::VideoCodec::H264, "mp4"},
        {QMediaFormat::MPEG4, QMediaFormat::VideoCodec::MPEG4, "mp4"},
        {QMediaFormat::AVI, QMediaFormat::VideoCodec::MotionJPEG, "avi"},
        {QMediaFormat::WMV, QMediaFormat::VideoCodec::WMV, "wmv"},
        {QMediaFormat::Matroska, QMediaFormat::VideoCodec::H264, "mkv"},
        {QMediaFormat::WebM, QMediaFormat::VideoCodec::VP8, "webm"},
    };

    for (const FormatOption& option : options) {
        QMediaFormat candidate(option.container);
        candidate.setVideoCodec(option.codec);
        if (candidate.isSupported(QMediaFormat::Encode)) {
            selectedFormat = candidate;
            fileExtension = QString::fromLatin1(option.extension);
            return true;
        }
    }
    return false;
}

QString ensureDefaultSaveDirectory() {
    const QString dirPath = QDir::current().filePath(QStringLiteral("captures"));
    QDir dir(dirPath);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    return QDir(dirPath).absolutePath();
}

}  // namespace

class DirectShowCameraControls final {
public:
    ~DirectShowCameraControls() {
#ifdef Q_OS_WIN
        release(videoProcAmp_);
        release(cameraControl_);
        release(filter_);
        if (uninitializeCom_) {
            CoUninitialize();
        }
#endif
    }

    bool open(const QByteArray& deviceId, const QString& deviceName, QString* error) {
#ifdef Q_OS_WIN
        const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(comResult)) {
            uninitializeCom_ = true;
        } else if (comResult != RPC_E_CHANGED_MODE) {
            if (error) {
                *error = QStringLiteral("初始化相机控制接口失败(0x%1)")
                             .arg(static_cast<quint32>(comResult), 8, 16, QLatin1Char('0'));
            }
            return false;
        }

        ICreateDevEnum* deviceEnumerator = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_ICreateDevEnum,
                                      reinterpret_cast<void**>(&deviceEnumerator));
        if (FAILED(hr) || deviceEnumerator == nullptr) {
            if (error) {
                *error = QStringLiteral("无法创建 Windows 相机设备枚举器。");
            }
            return false;
        }

        IEnumMoniker* monikerEnumerator = nullptr;
        hr = deviceEnumerator->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
                                                       &monikerEnumerator, 0);
        deviceEnumerator->Release();
        if (hr != S_OK || monikerEnumerator == nullptr) {
            if (error) {
                *error = QStringLiteral("DirectShow 未发现可控制的相机设备。");
            }
            return false;
        }

        const QString wantedId = normalizeDeviceId(QString::fromUtf8(deviceId));
        const QString wantedName = deviceName.trimmed();
        IMoniker* matchedMoniker = nullptr;
        IMoniker* moniker = nullptr;
        ULONG fetched = 0;
        int sameNameCount = 0;
        IMoniker* uniqueNameMoniker = nullptr;
        while (monikerEnumerator->Next(1, &moniker, &fetched) == S_OK && fetched == 1) {
            IPropertyBag* propertyBag = nullptr;
            QString path;
            QString name;
            if (SUCCEEDED(moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                                 reinterpret_cast<void**>(&propertyBag))) &&
                propertyBag != nullptr) {
                VARIANT value;
                VariantInit(&value);
                if (SUCCEEDED(propertyBag->Read(L"DevicePath", &value, nullptr)) &&
                    value.vt == VT_BSTR && value.bstrVal != nullptr) {
                    path = QString::fromWCharArray(value.bstrVal, SysStringLen(value.bstrVal));
                }
                VariantClear(&value);
                VariantInit(&value);
                if (SUCCEEDED(propertyBag->Read(L"FriendlyName", &value, nullptr)) &&
                    value.vt == VT_BSTR && value.bstrVal != nullptr) {
                    name = QString::fromWCharArray(value.bstrVal, SysStringLen(value.bstrVal));
                }
                VariantClear(&value);
                propertyBag->Release();
            }

            const bool exactIdMatch = !wantedId.isEmpty() &&
                                      normalizeDeviceId(path) == wantedId;
            if (exactIdMatch) {
                matchedMoniker = moniker;
                moniker->AddRef();
                break;
            }
            if (!wantedName.isEmpty() && name.compare(wantedName, Qt::CaseInsensitive) == 0) {
                ++sameNameCount;
                if (uniqueNameMoniker != nullptr) {
                    uniqueNameMoniker->Release();
                }
                uniqueNameMoniker = moniker;
                uniqueNameMoniker->AddRef();
            }
            moniker->Release();
            moniker = nullptr;
        }
        if (moniker != nullptr) {
            moniker->Release();
        }
        monikerEnumerator->Release();

        if (matchedMoniker == nullptr && sameNameCount == 1) {
            matchedMoniker = uniqueNameMoniker;
            uniqueNameMoniker = nullptr;
        }
        if (uniqueNameMoniker != nullptr) {
            uniqueNameMoniker->Release();
        }
        if (matchedMoniker == nullptr) {
            if (error) {
                *error = QStringLiteral("无法将相机“%1”与 Windows 图像控制设备安全匹配。")
                             .arg(deviceName);
            }
            return false;
        }

        hr = matchedMoniker->BindToObject(nullptr, nullptr, IID_IBaseFilter,
                                          reinterpret_cast<void**>(&filter_));
        matchedMoniker->Release();
        if (FAILED(hr) || filter_ == nullptr) {
            if (error) {
                *error = QStringLiteral("打开相机图像控制接口失败(0x%1)。")
                             .arg(static_cast<quint32>(hr), 8, 16, QLatin1Char('0'));
            }
            return false;
        }

        filter_->QueryInterface(IID_IAMVideoProcAmp,
                                reinterpret_cast<void**>(&videoProcAmp_));
        filter_->QueryInterface(IID_IAMCameraControl,
                                reinterpret_cast<void**>(&cameraControl_));
        if (videoProcAmp_ == nullptr && cameraControl_ == nullptr) {
            if (error) {
                *error = QStringLiteral("相机未提供标准 UVC 图像/镜头控制接口。");
            }
            return false;
        }
        return true;
#else
        Q_UNUSED(deviceId);
        Q_UNUSED(deviceName);
        if (error) {
            *error = QStringLiteral("相机硬件参数控制目前仅支持 Windows。");
        }
        return false;
#endif
    }

    QList<CameraControlDescriptor> supportedControls() const {
        QList<CameraControlDescriptor> result;
#ifdef Q_OS_WIN
        for (const ControlSpec& spec : controlSpecs()) {
            CameraControlDescriptor descriptor;
            descriptor.id = QString::fromLatin1(spec.id);
            descriptor.label = QString::fromUtf8(spec.label);
            descriptor.category = QString::fromUtf8(spec.category);
            long minimum = 0;
            long maximum = 0;
            long step = 0;
            long defaultValue = 0;
            long capabilities = 0;
            long value = 0;
            long flags = 0;
            HRESULT hr = E_NOINTERFACE;
            if (spec.isCameraControl && cameraControl_ != nullptr) {
                hr = cameraControl_->GetRange(spec.property, &minimum, &maximum, &step,
                                              &defaultValue, &capabilities);
                if (SUCCEEDED(hr)) {
                    cameraControl_->Get(spec.property, &value, &flags);
                }
                descriptor.supportsAuto = (capabilities & CameraControl_Flags_Auto) != 0;
                descriptor.supportsManual = (capabilities & CameraControl_Flags_Manual) != 0;
                descriptor.autoMode = (flags & CameraControl_Flags_Auto) != 0;
            } else if (!spec.isCameraControl && videoProcAmp_ != nullptr) {
                hr = videoProcAmp_->GetRange(spec.property, &minimum, &maximum, &step,
                                             &defaultValue, &capabilities);
                if (SUCCEEDED(hr)) {
                    videoProcAmp_->Get(spec.property, &value, &flags);
                }
                descriptor.supportsAuto = (capabilities & VideoProcAmp_Flags_Auto) != 0;
                descriptor.supportsManual = (capabilities & VideoProcAmp_Flags_Manual) != 0;
                descriptor.autoMode = (flags & VideoProcAmp_Flags_Auto) != 0;
            }
            if (FAILED(hr)) {
                continue;
            }
            descriptor.minimum = static_cast<int>(minimum);
            descriptor.maximum = static_cast<int>(maximum);
            descriptor.step = qMax(1, static_cast<int>(step));
            descriptor.defaultValue = static_cast<int>(defaultValue);
            descriptor.currentValue = static_cast<int>(value);
            result.append(descriptor);
        }
#endif
        return result;
    }

    bool set(const QString& id, int value, bool autoMode, QString* error) {
#ifdef Q_OS_WIN
        for (const ControlSpec& spec : controlSpecs()) {
            if (id != QLatin1String(spec.id)) {
                continue;
            }
            long capabilities = 0;
            long minimum = 0;
            long maximum = 0;
            long step = 0;
            long defaultValue = 0;
            HRESULT hr = E_NOINTERFACE;
            if (spec.isCameraControl && cameraControl_ != nullptr) {
                hr = cameraControl_->GetRange(spec.property, &minimum, &maximum, &step,
                                              &defaultValue, &capabilities);
                if (SUCCEEDED(hr)) {
                    const long flags = autoMode ? CameraControl_Flags_Auto
                                                : CameraControl_Flags_Manual;
                    hr = cameraControl_->Set(spec.property, value, flags);
                }
            } else if (!spec.isCameraControl && videoProcAmp_ != nullptr) {
                hr = videoProcAmp_->GetRange(spec.property, &minimum, &maximum, &step,
                                             &defaultValue, &capabilities);
                if (SUCCEEDED(hr)) {
                    const long flags = autoMode ? VideoProcAmp_Flags_Auto
                                                : VideoProcAmp_Flags_Manual;
                    hr = videoProcAmp_->Set(spec.property, value, flags);
                }
            }
            if (SUCCEEDED(hr)) {
                return true;
            }
            if (error) {
                *error = QStringLiteral("设置相机参数“%1”失败(0x%2)。")
                             .arg(QString::fromUtf8(spec.label))
                             .arg(static_cast<quint32>(hr), 8, 16, QLatin1Char('0'));
            }
            return false;
        }
        if (error) {
            *error = QStringLiteral("当前相机不支持此参数。");
        }
        return false;
#else
        Q_UNUSED(id);
        Q_UNUSED(value);
        Q_UNUSED(autoMode);
        if (error) {
            *error = QStringLiteral("相机硬件参数控制目前仅支持 Windows。");
        }
        return false;
#endif
    }

private:
#ifdef Q_OS_WIN
    struct ControlSpec {
        const char* id;
        const char* label;
        const char* category;
        long property;
        bool isCameraControl;
    };

    static const QList<ControlSpec>& controlSpecs() {
        static const QList<ControlSpec> specs = {
            {"brightness", "亮度", "颜色调整", VideoProcAmp_Brightness, false},
            {"contrast", "对比度", "颜色调整", VideoProcAmp_Contrast, false},
            {"hue", "色调", "颜色调整", VideoProcAmp_Hue, false},
            {"saturation", "饱和度", "颜色调整", VideoProcAmp_Saturation, false},
            {"sharpness", "锐度", "颜色调整", VideoProcAmp_Sharpness, false},
            {"gamma", "伽马", "颜色调整", VideoProcAmp_Gamma, false},
            {"color_enable", "彩色开关", "图像", VideoProcAmp_ColorEnable, false},
            {"white_balance", "白平衡", "白平衡", VideoProcAmp_WhiteBalance, false},
            {"backlight_compensation", "背光补偿", "曝光与增益", VideoProcAmp_BacklightCompensation, false},
            {"gain", "增益", "曝光与增益", VideoProcAmp_Gain, false},
            {"exposure", "曝光", "曝光与增益", CameraControl_Exposure, true},
            {"iris", "光圈", "曝光与增益", CameraControl_Iris, true},
            {"focus", "焦距", "镜头", CameraControl_Focus, true},
            {"zoom", "变焦", "镜头", CameraControl_Zoom, true},
            {"pan", "水平转动", "镜头", CameraControl_Pan, true},
            {"tilt", "垂直转动", "镜头", CameraControl_Tilt, true},
            {"roll", "旋转", "镜头", CameraControl_Roll, true},
        };
        return specs;
    }

    static QString normalizeDeviceId(QString id) {
        id = id.trimmed().toUpper();
        id.replace(QLatin1Char('/'), QLatin1Char('\\'));
        while (id.startsWith(QStringLiteral("\\\\?\\"))) {
            id.remove(0, 4);
        }
        const int guidSuffix = id.lastIndexOf(QStringLiteral("#{"));
        if (guidSuffix >= 0) {
            id.truncate(guidSuffix);
        }
        return id;
    }

    template <typename T>
    static void release(T*& object) {
        if (object != nullptr) {
            object->Release();
            object = nullptr;
        }
    }

    bool uninitializeCom_ = false;
    IBaseFilter* filter_ = nullptr;
    IAMVideoProcAmp* videoProcAmp_ = nullptr;
    IAMCameraControl* cameraControl_ = nullptr;
#endif
};

class CameraPreviewWidget final : public QWidget {
public:
    explicit CameraPreviewWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(200, 150);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setStyleSheet(QStringLiteral("background:#1f1f1f;border:1px solid #555;"));
    }

    void setFrame(const QImage& frame) {
        frame_ = frame;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);
        if (frame_.isNull()) {
            return;
        }
        const QSize scaled = frame_.size().scaled(size(), Qt::KeepAspectRatio);
        const QRect target(QPoint((width() - scaled.width()) / 2,
                                  (height() - scaled.height()) / 2), scaled);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawImage(target, frame_);
    }

private:
    QImage frame_;
};

QString CameraResolution::description() const {
    return QStringLiteral("%1 × %2").arg(width).arg(height);
}

CameraService::CameraService(QObject* parent)
    : QObject(parent),
      saveDirectory_(ensureDefaultSaveDirectory()),
      captureSession_(new QMediaCaptureSession(this)),
      videoSink_(new QVideoSink(this)),
      videoWidget_(new CameraPreviewWidget()),
      durationTimer_(new QTimer(this)) {

    videoWidget_->setObjectName(QStringLiteral("cameraPreview"));
    captureSession_->setVideoSink(videoSink_);
    connect(videoSink_, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame& frame) {
        const QImage image = transformCameraImage(frame.toImage(), previewMode_,
                                                  previewFlipHorizontal_, previewFlipVertical_);
        videoWidget_->setFrame(image);
    });

    durationTimer_->setTimerType(Qt::PreciseTimer);
    durationTimer_->setInterval(100);
    connect(durationTimer_, &QTimer::timeout, this, [this]() {
        if (recording_) {
            emit recordingDurationChanged(recordingElapsed_.elapsed());
        }
    });
}

CameraService::~CameraService() {
    closeCamera();
    delete videoWidget_;
}

void CameraService::refreshCameras() {
    cameraNames_.clear();
    cameraDevices_.clear();

    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const QCameraDevice& device : cameras) {
        cameraDevices_.push_back(device);
        cameraNames_.push_back(device.description());
    }

    emit cameraListChanged(cameraNames_);
    emit cameraLog(QStringLiteral("检测到相机数量: %1").arg(cameraNames_.size()));
}

QStringList CameraService::cameraNames() const {
    return cameraNames_;
}

QWidget* CameraService::previewWidget() const {
    return videoWidget_;
}

bool CameraService::openCamera(int index, QString* error) {
    if (index < 0 || index >= cameraDevices_.size()) {
        if (error) {
            *error = QStringLiteral("相机索引无效。");
        }
        return false;
    }

    closeCamera();

    const QCameraDevice& device = cameraDevices_.at(index);
    camera_ = new QCamera(device, this);
    const QList<QCameraFormat> formats = device.videoFormats();

    // Prefer a common preview size and choose its highest advertised frame rate.
    const QList<QSize> startupPreference = {
        QSize(1920, 1080), QSize(1280, 800), QSize(1280, 720), QSize(1024, 768),
        QSize(800, 600), QSize(640, 480)};
    QCameraFormat startupFormat;
    bool startupFormatFound = false;
    for (const QSize& preferred : startupPreference) {
        for (const QCameraFormat& candidate : formats) {
            if (candidate.resolution() == preferred &&
                (!startupFormatFound || candidate.maxFrameRate() > startupFormat.maxFrameRate())) {
                startupFormat = candidate;
                startupFormatFound = true;
            }
        }
        if (startupFormatFound) {
            break;
        }
    }
    if (!startupFormatFound) {
        for (const QCameraFormat& candidate : formats) {
            if (!startupFormatFound || candidate.maxFrameRate() > startupFormat.maxFrameRate()) {
                startupFormat = candidate;
                startupFormatFound = true;
            }
        }
    }
    if (startupFormatFound) {
        camera_->setCameraFormat(startupFormat);
    }

    imageCapture_ = new QImageCapture(this);
    recorder_ = new QMediaRecorder(this);

    captureSession_->setCamera(camera_);
    captureSession_->setImageCapture(imageCapture_);
    captureSession_->setRecorder(recorder_);

    // 日志：支持的格式
    emit cameraLog(QStringLiteral("相机支持 %1 种格式配置").arg(formats.size()));

    // 连接信号
    connect(camera_, &QCamera::errorOccurred, this,
            [this](QCamera::Error error, const QString& errorString) {
                Q_UNUSED(error);
                emit cameraLog(QStringLiteral("相机错误: %1").arg(errorString));
            });

    connect(imageCapture_, &QImageCapture::imageSaved, this,
            [this](int id, const QString& filePath) {
                Q_UNUSED(id);
                if (previewMode_ != 0 || previewFlipHorizontal_ || previewFlipVertical_) {
                    const QImage image(filePath);
                    const QImage transformed = transformCameraImage(
                        image, previewMode_, previewFlipHorizontal_, previewFlipVertical_);
                    if (!transformed.isNull() && !transformed.save(filePath, nullptr, 100)) {
                        emit cameraLog(QStringLiteral("黑白照片处理失败: %1").arg(filePath));
                    }
                }
                emit cameraLog(QStringLiteral("拍照保存成功: %1")
                    .arg(QDir::toNativeSeparators(filePath)));
            });

    connect(imageCapture_, &QImageCapture::errorOccurred, this,
            [this](int id, QImageCapture::Error error, const QString& errorString) {
                Q_UNUSED(id);
                Q_UNUSED(error);
                emit cameraLog(QStringLiteral("拍照失败: %1").arg(errorString));
            });

    connect(recorder_, &QMediaRecorder::recorderStateChanged, this,
            [this](QMediaRecorder::RecorderState state) {
                const bool newRecording = (state == QMediaRecorder::RecordingState);
                if (recording_ != newRecording) {
                    recording_ = newRecording;
                    emit recordingStateChanged(recording_, currentRecordPath_);
                    emit cameraLog(recording_ ? QStringLiteral("录像开始") : QStringLiteral("录像停止"));
                    if (recording_) {
                        startDurationTimer();
                    } else {
                        stopDurationTimer();
                        const QString stoppedPath = currentRecordPath_;
                        QTimer::singleShot(500, this, [this, stoppedPath]() {
                            const QFileInfo fileInfo(stoppedPath);
                            if (!fileInfo.exists() || fileInfo.size() < 1024) {
                                emit cameraLog(QStringLiteral(
                                    "录像文件异常：文件不存在或没有有效视频数据（%1）。")
                                                   .arg(QDir::toNativeSeparators(stoppedPath)));
                                return;
                            }
                            emit cameraLog(QStringLiteral("录像文件已保存：%1（%2 MB）")
                                               .arg(QDir::toNativeSeparators(stoppedPath))
                                               .arg(fileInfo.size() / (1024.0 * 1024.0), 0, 'f', 2));
                        });
                    }
                }
            });

    connect(recorder_, &QMediaRecorder::errorOccurred, this,
            [this](QMediaRecorder::Error error, const QString& errorString) {
                Q_UNUSED(error);
                emit cameraLog(QStringLiteral("录像错误: %1").arg(errorString));
            });

    camera_->start();

    if (!camera_->isActive()) {
        if (error) {
            *error = QStringLiteral("相机启动失败");
        }
        closeCamera();
        return false;
    }

    opened_ = true;
    currentIndex_ = index;
    updateResolutions();

    directShowControls_ = std::make_unique<DirectShowCameraControls>();
    QString controlError;
    if (directShowControls_->open(device.id(), device.description(), &controlError)) {
        const int controlCount = directShowControls_->supportedControls().size();
        emit cameraLog(QStringLiteral("相机标准控制接口已连接，可调参数 %1 项。").arg(controlCount));
    } else {
        emit cameraLog(QStringLiteral("相机图像控制接口不可用: %1").arg(controlError));
        directShowControls_.reset();
    }

    emit cameraStateChanged(true, device.description());
    emit cameraLog(QStringLiteral("相机已打开: %1").arg(device.description()));
    return true;
}

void CameraService::closeCamera() {
    const bool wasOpen = opened_;

    stopRecording();
    directShowControls_.reset();

    if (camera_) {
        camera_->stop();
        camera_->deleteLater();
        camera_ = nullptr;
    }

    if (imageCapture_) {
        imageCapture_->deleteLater();
        imageCapture_ = nullptr;
    }

    if (recorder_) {
        captureSession_->setRecorder(nullptr);
        recorder_->deleteLater();
        recorder_ = nullptr;
    }

    opened_ = false;
    currentIndex_ = -1;
    recording_ = false;
    supportedResolutions_.clear();

    if (wasOpen) {
        emit cameraStateChanged(false, QString());
        emit cameraLog(QStringLiteral("相机已关闭"));
    }
}

bool CameraService::isOpen() const {
    return opened_;
}

int CameraService::currentIndex() const {
    return currentIndex_;
}

QString CameraService::currentCameraName() const {
    if (currentIndex_ >= 0 && currentIndex_ < cameraNames_.size()) {
        return cameraNames_.at(currentIndex_);
    }
    return QString();
}

QString CameraService::currentCameraId() const {
    if (currentIndex_ >= 0 && currentIndex_ < cameraDevices_.size()) {
        return QString::fromUtf8(cameraDevices_.at(currentIndex_).id());
    }
    return QString();
}

QList<CameraResolution> CameraService::supportedResolutions() const {
    return supportedResolutions_;
}

CameraResolution CameraService::currentResolution() const {
    if (!camera_ || !opened_) {
        return {};
    }

    const QCameraFormat fmt = camera_->cameraFormat();
    CameraResolution res;
    res.width = fmt.resolution().width();
    res.height = fmt.resolution().height();
    res.minFps = fmt.minFrameRate();
    res.maxFps = fmt.maxFrameRate();
    return res;
}

bool CameraService::setResolution(int width, int height, QString* error) {
    if (!camera_ || !opened_) {
        if (error) {
            *error = QStringLiteral("相机未打开");
        }
        return false;
    }

    const QCameraDevice& device = cameraDevices_.at(currentIndex_);
    const QList<QCameraFormat> formats = device.videoFormats();

    QCameraFormat bestFormat;
    bool found = false;

    for (const QCameraFormat& fmt : formats) {
        if (fmt.resolution().width() == width && fmt.resolution().height() == height) {
            if (!found || fmt.maxFrameRate() > bestFormat.maxFrameRate()) {
                bestFormat = fmt;
                found = true;
            }
        }
    }

    if (!found) {
        if (error) {
            *error = QStringLiteral("不支持的分辨率: %1x%2").arg(width).arg(height);
        }
        return false;
    }

    camera_->setCameraFormat(bestFormat);
    emit resolutionChanged(width, height);
    return true;
}

bool CameraService::setFrameRate(double fps, QString* error) {
    if (!camera_ || !opened_) {
        if (error) {
            *error = QStringLiteral("相机未打开");
        }
        return false;
    }

    QCameraFormat fmt = camera_->cameraFormat();
    if (fps < fmt.minFrameRate() || fps > fmt.maxFrameRate()) {
        if (error) {
            *error = QStringLiteral("帧率超出范围 [%1, %2]")
                .arg(fmt.minFrameRate()).arg(fmt.maxFrameRate());
        }
        return false;
    }

    // Qt6没有直接设置帧率的API；当前采集格式在选择分辨率时已采用该尺寸最高帧率模式。
    emit cameraLog(QStringLiteral("当前采集帧率范围: %1-%2 FPS")
        .arg(fmt.minFrameRate()).arg(fmt.maxFrameRate()));
    return true;
}

bool CameraService::captureImage(QString* savedPath, QString* error) {
    if (!opened_ || !imageCapture_ || !camera_) {
        if (error) {
            *error = QStringLiteral("相机未打开，无法拍照");
        }
        return false;
    }

    if (!imageCapture_->isReadyForCapture()) {
        if (error) {
            *error = QStringLiteral("相机当前不可拍照，请稍后重试");
        }
        return false;
    }

    const QString path = buildCapturePath();
    imageCapture_->captureToFile(path);

    if (savedPath) {
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
    emit cameraLog(QStringLiteral("保存目录: %1").arg(QDir::toNativeSeparators(saveDirectory_)));
}

QString CameraService::saveDirectory() const {
    return saveDirectory_;
}

bool CameraService::startRecording(QString* savedPath, QString* error) {
    if (!opened_ || !camera_ || !recorder_) {
        if (error) {
            *error = QStringLiteral("相机未打开，无法录像");
        }
        return false;
    }

    if (recording_) {
        if (savedPath) {
            *savedPath = currentRecordPath_;
        }
        return true;
    }

    QDir dir(saveDirectory_.isEmpty() ? ensureDefaultSaveDirectory() : saveDirectory_);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    QMediaFormat format;
    QString fileExtension;
    if (!chooseSupportedVideoFormat(format, fileExtension)) {
        if (error) {
            *error = QStringLiteral("当前系统没有可用的相机录像编码格式，未创建录像文件。");
        }
        emit cameraLog(QStringLiteral("录像启动失败：没有可用的容器/视频编码组合。"));
        return false;
    }

    currentRecordPath_ = buildRecordPath(fileExtension);
    recorder_->setMediaFormat(format);
    recorder_->setOutputLocation(QUrl::fromLocalFile(currentRecordPath_));
    recorder_->setQuality(QMediaRecorder::HighQuality);
    captureSession_->setRecorder(recorder_);

    emit cameraLog(QStringLiteral("录像启动请求：%1（%2 / %3）")
                       .arg(QDir::toNativeSeparators(currentRecordPath_))
                       .arg(QMediaFormat::fileFormatDescription(format.fileFormat()))
                       .arg(QMediaFormat::videoCodecDescription(format.videoCodec())));

    recorder_->record();

    if (savedPath) {
        *savedPath = currentRecordPath_;
    }

    return true;
}

void CameraService::stopRecording() {
    if (recorder_ && recorder_->recorderState() == QMediaRecorder::RecordingState) {
        recorder_->stop();
    }
}

bool CameraService::isRecording() const {
    return recording_;
}

QList<CameraControlDescriptor> CameraService::supportedControls() const {
    return directShowControls_ != nullptr
               ? directShowControls_->supportedControls()
               : QList<CameraControlDescriptor>();
}

bool CameraService::setCameraControl(const QString& id, int value, bool autoMode, QString* error) {
    if (!opened_ || directShowControls_ == nullptr) {
        if (error) {
            *error = QStringLiteral("当前相机没有可用的硬件参数控制接口。");
        }
        return false;
    }
    return directShowControls_->set(id, value, autoMode, error);
}

void CameraService::setMonochromePreview(bool monochrome) {
    monochromePreview_ = monochrome;
    previewMode_ = monochrome ? 1 : 0;
}

bool CameraService::monochromePreview() const {
    return previewMode_ == 1;
}

void CameraService::setPreviewTransform(int mode, bool flipHorizontal, bool flipVertical) {
    previewMode_ = qBound(0, mode, 2);
    monochromePreview_ = previewMode_ == 1;
    previewFlipHorizontal_ = flipHorizontal;
    previewFlipVertical_ = flipVertical;
}

bool CameraService::applyImageSettings(double brightness, double contrast, double saturation,
                                       double exposureCompensation, bool monochrome,
                                       QString* error) {
    const QList<QPair<QString, int>> values = {
        {QStringLiteral("brightness"), qRound(brightness)},
        {QStringLiteral("contrast"), qRound(contrast)},
        {QStringLiteral("saturation"), qRound(saturation)},
        {QStringLiteral("exposure"), qRound(exposureCompensation)},
        {QStringLiteral("color_enable"), monochrome ? 0 : 1},
    };
    for (const auto& value : values) {
        QString controlError;
        if (!setCameraControl(value.first, value.second, false, &controlError)) {
            if (error != nullptr) {
                *error = controlError;
            }
            return false;
        }
    }
    return true;
}

bool CameraService::readImageSettings(double& brightness, double& contrast, double& saturation,
                                      double& exposureCompensation, bool& monochrome,
                                      QString* error) const {
    if (!opened_ || directShowControls_ == nullptr) {
        if (error) {
            *error = QStringLiteral("当前相机没有可用的硬件参数控制接口。");
        }
        return false;
    }
    const auto controls = directShowControls_->supportedControls();
    auto read = [&controls](const QString& id, double& value, bool* autoMode = nullptr) {
        const auto it = std::find_if(controls.cbegin(), controls.cend(),
                                     [&id](const CameraControlDescriptor& item) {
                                         return item.id == id;
                                     });
        if (it == controls.cend()) {
            return false;
        }
        value = it->currentValue;
        if (autoMode != nullptr) {
            *autoMode = it->autoMode;
        }
        return true;
    };
    if (!read(QStringLiteral("brightness"), brightness) ||
        !read(QStringLiteral("contrast"), contrast) ||
        !read(QStringLiteral("saturation"), saturation) ||
        !read(QStringLiteral("exposure"), exposureCompensation)) {
        if (error) {
            *error = QStringLiteral("相机没有完整支持亮度、对比度、饱和度和曝光参数。");
        }
        return false;
    }
    double colorEnabled = 1.0;
    if (!read(QStringLiteral("color_enable"), colorEnabled)) {
        monochrome = false;
    } else {
        monochrome = qRound(colorEnabled) == 0;
    }
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

QString CameraService::buildRecordPath(const QString& extension) const {
    QDir dir(saveDirectory_.isEmpty() ? ensureDefaultSaveDirectory() : saveDirectory_);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    const QString timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss_zzz"));
    return dir.filePath(QStringLiteral("video_%1.%2").arg(timestamp, extension));
}

void CameraService::updateResolutions() {
    supportedResolutions_.clear();

    if (currentIndex_ < 0 || currentIndex_ >= cameraDevices_.size()) {
        return;
    }

    const QCameraDevice& device = cameraDevices_.at(currentIndex_);
    const QList<QCameraFormat> formats = device.videoFormats();

    for (const QCameraFormat& fmt : formats) {
        const int w = fmt.resolution().width();
        const int h = fmt.resolution().height();
        auto existing = std::find_if(supportedResolutions_.begin(), supportedResolutions_.end(),
                                     [w, h](const CameraResolution& item) {
            return item.width == w && item.height == h;
        });
        if (existing == supportedResolutions_.end()) {
            CameraResolution resolution;
            resolution.width = w;
            resolution.height = h;
            resolution.minFps = fmt.minFrameRate();
            resolution.maxFps = fmt.maxFrameRate();
            supportedResolutions_.append(resolution);
        } else {
            existing->minFps = qMin(existing->minFps, fmt.minFrameRate());
            existing->maxFps = qMax(existing->maxFps, fmt.maxFrameRate());
        }
    }

    // 按分辨率降序排序
    std::sort(supportedResolutions_.begin(), supportedResolutions_.end(),
              [](const CameraResolution& a, const CameraResolution& b) {
                  return (a.width * a.height) > (b.width * b.height);
              });

    // 只展示设备实际支持的常用分辨率，顺序与常见相机工具一致。
    const QList<QSize> commonResolutions = {
        QSize(800, 600),   QSize(1024, 768), QSize(1280, 800), QSize(1280, 720),
        QSize(1600, 1200), QSize(1920, 1080), QSize(2048, 1536), QSize(2592, 1944),
        QSize(640, 480),
    };
    QList<CameraResolution> common;
    for (const QSize& preferred : commonResolutions) {
        const auto it = std::find_if(supportedResolutions_.cbegin(), supportedResolutions_.cend(),
                                     [&preferred](const CameraResolution& resolution) {
            return resolution.width == preferred.width() && resolution.height == preferred.height();
        });
        if (it != supportedResolutions_.cend()) {
            common.append(*it);
        }
    }
    const CameraResolution active = currentResolution();
    const bool activeIncluded = std::any_of(
        common.cbegin(), common.cend(), [&active](const CameraResolution& resolution) {
            return resolution.width == active.width && resolution.height == active.height;
        });
    if (!activeIncluded && active.width > 0 && active.height > 0) {
        common.prepend(active);
    }
    if (common.isEmpty()) {
        common = supportedResolutions_.mid(0, qMin(3, supportedResolutions_.size()));
    }
    supportedResolutions_ = common;

    emit cameraLog(QStringLiteral("支持 %1 种分辨率配置").arg(supportedResolutions_.size()));
}

void CameraService::startDurationTimer() {
    recordingElapsed_.start();
    durationTimer_->start();
    emit recordingDurationChanged(0);
}

void CameraService::stopDurationTimer() {
    durationTimer_->stop();
    emit recordingDurationChanged(0);
}
