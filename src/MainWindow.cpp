#include "MainWindow.h"

#include "ui_MainWindow.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QPushButton>
#include <QShortcut>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

namespace {

constexpr int kFineHomeDiIndex = 1;
constexpr double kFineHomeJogSpeedScale = 0.5;

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      configManager_(QStringLiteral("config/app_config.ini")),
      logService_(QStringLiteral("logs")),
      syncPlaybackService_(&motionService_),
      ui_(std::make_unique<Ui::MainWindow>()) {
    setupUi();
    initServices();
    wireSignals();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    ui_->setupUi(this);
    setWindowTitle(QStringLiteral("体积光固化旋转投影同步控制软件"));
    setMinimumSize(1600, 900);
    resize(1700, 960);
    setStyleSheet(QStringLiteral(
        "QWidget{font-family:'Microsoft YaHei UI';font-size:14px;color:#2d3640;}"
        "QMainWindow{background:#f3f5f8;}"
        "QGroupBox{font-weight:700;background:#ffffff;border:1px solid #d9dde3;border-radius:8px;margin-top:12px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:12px;padding:0 6px;color:#1f2937;}"
        "QLabel{color:#2d3640;}"
        "QPushButton{min-width:96px;min-height:36px;padding:0 14px;background:#f7f9fc;border:1px solid #c9d1da;border-radius:6px;}"
        "QPushButton:hover{background:#eef3fa;}"
        "QPushButton:pressed{background:#e2eaf5;}"
        "QPushButton[role=\"primary\"]{background:#2f80ed;color:white;border:1px solid #2a70cf;}"
        "QPushButton[role=\"primary\"]:hover{background:#2b76dd;}"
        "QPushButton[role=\"primary\"]:pressed{background:#2567c3;}"
        "QPushButton[role=\"danger\"]{background:#e53935;color:white;border:1px solid #b52a28;font-weight:700;}"
        "QPushButton[role=\"danger\"]:hover{background:#d7302d;}"
        "QPushButton[role=\"danger\"]:pressed{background:#be2623;}"
        "QLineEdit,QComboBox,QDoubleSpinBox,QSpinBox{min-height:36px;background:white;border:1px solid #c9d1da;border-radius:6px;padding:0 10px;}"
        "QPlainTextEdit{background:#ffffff;border:1px solid #c9d1da;border-radius:6px;}"));

    ui_->mainHorizontalLayout->setStretch(0, 4);
    ui_->mainHorizontalLayout->setStretch(1, 6);
    ui_->leftPanelLayout->setStretch(0, 4);
    ui_->leftPanelLayout->setStretch(1, 4);
    ui_->leftPanelLayout->setStretch(2, 5);
    ui_->rightPanelLayout->setStretch(0, 7);
    ui_->rightPanelLayout->setStretch(1, 3);
    ui_->cameraBodyLayout->setStretch(0, 3);
    ui_->cameraBodyLayout->setStretch(1, 1);

    btnConnectCard_ = ui_->btnConnectCard;
    comboBoardPort_ = ui_->comboBoardPort;
    btnBoardRefresh_ = ui_->btnBoardRefresh;
    lblCardStatus_ = ui_->lblCardStatus;
    lblComPort_ = ui_->lblComPort;
    diIndicators_ = {ui_->lblDi0, ui_->lblDi1};

    motionSectionWidget_ = ui_->motionControlsPanel;
    btnJogPlusMotion_ = ui_->btnJogPlus;
    btnJogMinusMotion_ = ui_->btnJogMinus;
    btnStopMotion_ = ui_->btnStop;
    btnEStopMotion_ = ui_->btnEStop;
    editJogSpeedPulseMs_ = ui_->editJogSpeedPulseMs;
    editTargetAbsDeg_ = ui_->editTargetAbsDeg;
    editTargetRelDeg_ = ui_->editTargetRelDeg;
    btnMoveAbsMotion_ = ui_->btnMoveAbs;
    btnMoveRelMotion_ = ui_->btnMoveRel;
    lblCurrentDeg_ = ui_->lblCurrentDeg;
    lblCurrentPulse_ = ui_->lblCurrentPulse;
    lblMotionConfig_ = ui_->lblMotionConfig;
    btnCoarseCheck_ = ui_->btnCoarseCheck;
    btnFineHome_ = ui_->btnFineHome;
    btnConfirmZReady_ = ui_->btnConfirmZReady;
    lblLaserState_ = ui_->lblLaserState;
    lblHomeResult_ = ui_->lblHomeResult;
    lblZeroErrorDeg_ = ui_->lblZeroErrorDeg;
    lblZConfirmTime_ = ui_->lblZConfirmTime;

    comboCameraList_ = ui_->comboCameraList;
    btnCameraRefresh_ = ui_->btnCameraRefresh;
    btnCameraOpen_ = ui_->btnCameraOpen;
    lblCameraState_ = ui_->lblCameraState;
    lblCameraRecordTime_ = ui_->lblCameraRecordTime;
    btnSnap_ = ui_->btnSnap;
    btnRec_ = ui_->btnRec;
    btnCameraModeToggle_ = ui_->btnCameraModeToggle;
    editCameraSaveDir_ = ui_->editCameraSaveDir;
    spinCameraBrightness_ = ui_->spinCameraBrightness;
    spinCameraContrast_ = ui_->spinCameraContrast;
    spinCameraSaturation_ = ui_->spinCameraSaturation;
    spinCameraExposure_ = ui_->spinCameraExposure;

    comboProjectorPort_ = ui_->comboProjectorPort;
    btnProjectorRefresh_ = ui_->btnRefreshProjectorPort;
    btnProjectorOpen_ = ui_->btnProjectorOpen;
    btnProjectorClose_ = ui_->btnProjectorClose;
    lblProjectorPortState_ = ui_->lblProjectorPortState;
    btnProjectorPowerOn_ = ui_->btnProjectorPowerOn;
    btnProjectorPowerOff_ = ui_->btnProjectorPowerOff;
    btnLedOn_ = ui_->btnLedOn;
    btnLedOff_ = ui_->btnLedOff;
    editLedPercent_ = ui_->editLedPercent;
    btnApplyLedPercent_ = ui_->btnApplyLedPercent;
    btnAlignSiemens_ = ui_->btnAlignSiemens;
    editImageFolder_ = ui_->editImageFolder;
    btnBrowseImageFolder_ = ui_->btnBrowseImageFolder;
    btnCheckImageFolder_ = ui_->btnCheckImageFolder;
    lblImageCheckResult_ = ui_->lblImageCheckResult;
    spinLoopCount_ = ui_->spinLoopCount;
    chkInfiniteLoop_ = ui_->chkInfiniteLoop;
    editRoundTimeSec_ = ui_->editRoundTimeSec;
    lblCurrentFrame_ = ui_->lblCurrentFrame;
    lblSyncState_ = ui_->lblSyncState;
    lblSyncProgress_ = ui_->lblSyncProgress;
    lblSyncElapsed_ = ui_->lblSyncElapsed;
    btnSyncStart_ = ui_->btnSyncStart;
    btnSyncStop_ = ui_->btnSyncStop;
    lblProjectionPreview_ = ui_->lblProjectionPreview;
    textLog_ = ui_->textLog;

    btnConnectCard_->setProperty("role", QStringLiteral("primary"));
    btnConfirmZReady_->setProperty("role", QStringLiteral("primary"));
    btnSyncStart_->setProperty("role", QStringLiteral("primary"));
    btnEStopMotion_->setProperty("role", QStringLiteral("danger"));

    lblCardStatus_->setStyleSheet(
        QStringLiteral("QLabel{background:#eef4ff;border:1px solid #c8daf8;border-radius:6px;padding:4px 8px;}"));
    lblProjectorPortState_->setStyleSheet(
        QStringLiteral("QLabel{background:#eef4ff;border:1px solid #c8daf8;border-radius:6px;padding:4px 8px;}"));
    lblCameraState_->setStyleSheet(
        QStringLiteral("QLabel{background:#eef4ff;border:1px solid #c8daf8;border-radius:6px;padding:4px 8px;}"));
    lblCameraRecordTime_->setStyleSheet(
        QStringLiteral("QLabel{background:#f5f7fa;border:1px solid #dbe2ea;border-radius:6px;padding:4px 8px;}"));
    lblImageCheckResult_->setStyleSheet(
        QStringLiteral("QLabel{background:#fef3f2;border:1px solid #f5c2c7;color:#b42318;border-radius:6px;padding:4px 8px;font-weight:600;}"));
    lblLaserState_->setStyleSheet(
        QStringLiteral("QLabel{background:#f5f7fa;border:1px solid #dbe2ea;border-radius:6px;padding:4px 8px;}"));
    lblProjectionPreview_->setStyleSheet(
        QStringLiteral("QLabel{background:#111;border:1px solid #555;color:#ddd;border-radius:6px;}"));

    editJogSpeedPulseMs_->setSuffix(QStringLiteral(" °/s"));
    editTargetAbsDeg_->setSuffix(QStringLiteral(" °"));
    editTargetRelDeg_->setSuffix(QStringLiteral(" °"));
    editLedPercent_->setSuffix(QStringLiteral(" %"));
    editRoundTimeSec_->setSuffix(QStringLiteral(" s"));
    editImageFolder_->setPlaceholderText(
        QStringLiteral("选择图片文件夹（支持 0deg.png 或 0000.png）"));

    editCameraSaveDir_->setCursor(Qt::PointingHandCursor);
    editCameraSaveDir_->setToolTip(QStringLiteral("点击选择相机保存目录"));
    editCameraSaveDir_->installEventFilter(this);
    lblLaserState_->setText(QStringLiteral("对射状态: 未连接"));
    if (btnConfirmZReady_ != nullptr) {
        btnConfirmZReady_->setEnabled(false);
    }
    if (lblHomeResult_ != nullptr) {
        lblHomeResult_->setVisible(true);
    }
    if (lblZeroErrorDeg_ != nullptr) {
        lblZeroErrorDeg_->setVisible(true);
    }
    if (lblZConfirmTime_ != nullptr) {
        lblZConfirmTime_->setVisible(true);
    }

    QWidget* preview = cameraService_.previewWidget();
    preview->setObjectName(QStringLiteral("cameraPreviewWidget"));
    preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto* previewLayout = new QVBoxLayout(ui_->cameraPreviewHost);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->addWidget(preview);

    if (textLog_ != nullptr && textLog_->toPlainText().isEmpty()) {
        textLog_->appendPlainText(QStringLiteral("[信息] UI 骨架已初始化。"));
    }
}

void MainWindow::initServices() {
    logService_.setUiSink([this](const QString& message) {
        appendUiLog(message);
    });

    QString error;
    if (!logService_.initialize(&error)) {
        appendUiLog(QStringLiteral("[错误][系统] %1").arg(error));
    } else {
        logService_.info(QStringLiteral("系统"), QStringLiteral("日志服务初始化完成。"));
    }

    if (!configManager_.load(appConfig_, &error)) {
        appendUiLog(QStringLiteral("[错误][系统] %1").arg(error));
        appConfig_ = ConfigManager::defaultConfig();
    } else {
        logService_.info(QStringLiteral("系统"), QStringLiteral("配置加载完成。"));
    }
    laserTriggered_ = false;
    laserStableTimer_.start();

    motionService_.setPulsePerDeg(appConfig_.pulsePerDeg);
    motionService_.setMaxVelocityPulsePerMs(appConfig_.velMaxPulseMs);
    motionService_.setBoardAdapter(&boardAdapter_);
    motionService_.setAxisIndex(appConfig_.axisIndex);
    projectorService_.applyConfig(appConfig_);

    editRoundTimeSec_->setValue(20.0);
    editLedPercent_->setValue(20);
    const double defaultJogPulseMs =
        qBound(1.0, appConfig_.velMaxPulseMs * 0.8, appConfig_.velMaxPulseMs);
    const double defaultJogDegPerSec =
        defaultJogPulseMs * 1000.0 / appConfig_.pulsePerDeg;
    if (editJogSpeedPulseMs_ != nullptr) {
        editJogSpeedPulseMs_->setValue(defaultJogDegPerSec);
    }
    updateJogButtonStyle();
    if (lblMotionConfig_ != nullptr) {
        lblMotionConfig_->setText(
            QStringLiteral("参数: 电机%1脉冲/圈, 减速比1:%2, 当量%3脉冲/°")
                .arg(appConfig_.pulsePerMotorRev)
                .arg(appConfig_.gearRatio)
                .arg(appConfig_.pulsePerDeg, 0, 'f', 3));
    }
    lblSyncState_->setText(QStringLiteral("同步状态: READY"));
    lblSyncProgress_->setText(QStringLiteral("同步进度: 0/0"));
    lblCurrentFrame_->setText(QStringLiteral("当前投影: N/A"));
    if (lblCameraState_ != nullptr) {
        lblCameraState_->setText(QStringLiteral("相机状态: 未打开"));
    }
    const QString defaultCameraSaveDir = QDir::current().filePath(QStringLiteral("captures"));
    cameraService_.setSaveDirectory(defaultCameraSaveDir);
    if (editCameraSaveDir_ != nullptr) {
        editCameraSaveDir_->setText(QDir::toNativeSeparators(cameraService_.saveDirectory()));
    }
    if (lblCameraRecordTime_ != nullptr) {
        lblCameraRecordTime_->setText(QStringLiteral("录像时长: 00:00:00"));
    }
    cameraMonochrome_ = false;
    if (btnCameraModeToggle_ != nullptr) {
        btnCameraModeToggle_->setText(QStringLiteral("模式: 彩色"));
    }
    if (lblProjectionPreview_ != nullptr) {
        lblProjectionPreview_->setText(QStringLiteral("实时投影预览"));
    }

    btnSyncStop_->setEnabled(false);
    boardPortScanTimer_.setInterval(1500);
    coarseHomeTimer_.setInterval(5);
    coarseHomeTimer_.setTimerType(Qt::PreciseTimer);
    fineHomeTimer_.setInterval(5);
    fineHomeTimer_.setTimerType(Qt::PreciseTimer);
    syncElapsedUiTimer_.setInterval(200);
    refreshBoardPorts();
    refreshProjectorPorts();
    updateDiIndicators(0);
    updateBoardDependentUiState();
    updateSyncElapsedLabel(0);

    logService_.info(
        QStringLiteral("系统"),
        QStringLiteral("运动参数: axis=%1, pulse_per_motor_rev=%2, gear_ratio=1:%3, pulse_per_deg=%4, vel_max=%5 °/s")
            .arg(appConfig_.axisIndex)
            .arg(appConfig_.pulsePerMotorRev)
            .arg(appConfig_.gearRatio)
            .arg(appConfig_.pulsePerDeg, 0, 'f', 3)
            .arg(appConfig_.velMaxPulseMs * 1000.0 / appConfig_.pulsePerDeg, 0, 'f', 3));
    logService_.info(
        QStringLiteral("系统"),
        QStringLiteral("回零参数: di=%1, direction=%2, max_turns=%3")
            .arg(appConfig_.coarseHomeDiIndex)
            .arg(appConfig_.coarseHomeDirection > 0 ? QStringLiteral("+") : QStringLiteral("-"))
            .arg(appConfig_.coarseHomeMaxTurns, 0, 'f', 3));
    logService_.info(
        QStringLiteral("系统"),
        QStringLiteral("精零点参数: di=%1, active_low=%2, 扫描方向=+, 回中方向=-, max_travel=%3°, timeout=%4ms, stable=%5ms")
            .arg(kFineHomeDiIndex)
            .arg(appConfig_.fineHomeActiveLow ? QStringLiteral("true") : QStringLiteral("false"))
            .arg(appConfig_.fineHomeMaxTravelDeg, 0, 'f', 3)
            .arg(appConfig_.fineHomeTimeoutMs)
            .arg(appConfig_.laserStableMs));
}

void MainWindow::wireSignals() {
    connect(&boardAdapter_, &BoardAdapter::boardLog, this, [this](const QString& msg) {
        logService_.info(QStringLiteral("板卡"), msg);
    });
    connect(&boardAdapter_, &BoardAdapter::connectionChanged, this,
            [this](bool connected, const QString& portName) {
                lblCardStatus_->setText(
                    connected ? QStringLiteral("板卡状态: 已连接（%1）").arg(portName)
                              : QStringLiteral("板卡状态: 未连接"));
                if (lblComPort_ != nullptr) {
                    lblComPort_->setText(QStringLiteral("板卡串口: %1").arg(portName));
                }
                btnConnectCard_->setText(connected ? QStringLiteral("断开板卡")
                                                  : QStringLiteral("连接板卡"));
                if (connected) {
                    safetyTripped_ = false;
                    lastBoardPort_ = portName.trimmed().toUpper();
                } else {
                    if (syncPlaybackService_.isRunning()) {
                        stopSyncWorkflow(false);
                    }
                    if (coarseHoming_) {
                        finishCoarseHoming(false, QStringLiteral("粗零点中断: 板卡已断开。"));
                    } else if (fineHoming_) {
                        finishFineHoming(false, QStringLiteral("精零点中断: 板卡已断开。"));
                    } else if (motionService_.stateText() != QStringLiteral("READY")) {
                        motionService_.stop(true);
                    }
                }
                updateBoardDependentUiState();
            });
    connect(&boardAdapter_, &BoardAdapter::diChanged, this,
            [this](quint32 raw) {
                lastDiRaw_ = raw;
                updateDiIndicators(raw);
                const bool di2 = (raw >> 2) & 0x1U;
                const bool di3 = (raw >> 3) & 0x1U;
                if (di2) {
                    handleSafetyTrip(QStringLiteral("急停触发"));
                } else if (di3) {
                    handleSafetyTrip(QStringLiteral("伺服报警触发"));
                }
                if (coarseHoming_ && coarseHomeStage_ == CoarseHomeStage::SeekEdge) {
                    const bool di = (raw >> appConfig_.coarseHomeDiIndex) & 0x1U;
                    if (di != coarseHomeInitialDi_) {
                        motionService_.refreshPosition();
                        coarseHomeFirstEdgePulse_ = motionService_.currentPulse();
                        motionService_.stop(true);
                        coarseHomeStage_ = CoarseHomeStage::StopSettle;
                        coarseHomeConfirmRemainMs_ = 40;
                        logService_.info(
                            QStringLiteral("回零"),
                            QStringLiteral("粗零点边沿触发，位置脉冲: %1")
                                .arg(coarseHomeFirstEdgePulse_));
                    }
                }
                if (fineHoming_) {
                    const bool laserTriggered = laserTriggeredFromRaw(raw);
                    if (laserTriggered != fineHomeLastLaserState_) {
                        motionService_.refreshPosition();
                        const qint64 pulse = motionService_.currentPulse();
                        fineHomeLastLaserState_ = laserTriggered;
                        if (fineHomeStage_ == FineHomeStage::SeekFirstEdge && laserTriggered) {
                            fineHomeFirstEdgePulse_ = pulse;
                            fineHomeStage_ = FineHomeStage::SeekSecondEdge;
                            logService_.info(
                                QStringLiteral("回零"),
                                QStringLiteral("精零点进入光束: 脉冲=%1。")
                                    .arg(fineHomeFirstEdgePulse_));
                        } else if (fineHomeStage_ == FineHomeStage::SeekSecondEdge && !laserTriggered) {
                            fineHomeSecondEdgePulse_ = pulse;
                            fineHomeMidPulse_ =
                                static_cast<qint64>((fineHomeFirstEdgePulse_ + fineHomeSecondEdgePulse_) / 2);
                            motionService_.stop(false);
                            fineHomeStage_ = FineHomeStage::MoveToMidpoint;
                            const double midDeg = static_cast<double>(fineHomeMidPulse_) / appConfig_.pulsePerDeg;
                            if (!motionService_.moveToAbsDeg(midDeg)) {
                                finishFineHoming(false, QStringLiteral("精零点失败: 无法反向回到中点。"));
                                return;
                            }
                            logService_.info(
                                QStringLiteral("回零"),
                                QStringLiteral("精零点离开光束: 脉冲=%1。")
                                    .arg(fineHomeSecondEdgePulse_)
                            );
                            logService_.info(
                                QStringLiteral("回零"),
                                QStringLiteral("精零点回中: P1=%1, P2=%2, P0=%3, 目标角度=%4°")
                                    .arg(fineHomeFirstEdgePulse_)
                                    .arg(fineHomeSecondEdgePulse_)
                                    .arg(fineHomeMidPulse_)
                                    .arg(midDeg, 0, 'f', 3));
                        }
                    }
                }
            });

    connect(&motionService_, &MotionService::motionLog, this,
            [this](const QString& msg) { logService_.info(QStringLiteral("运动"), msg); });
    connect(&motionService_, &MotionService::motionError, this,
            [this](const QString& msg) { logService_.error(QStringLiteral("运动"), msg); });
    connect(&motionService_, &MotionService::positionChanged, this,
            [this](qint64 pulse, double, double cycleDeg) {
                lblCurrentPulse_->setText(QStringLiteral("当前位置脉冲: %1").arg(pulse));
                lblCurrentDeg_->setText(QStringLiteral("当前角度: %1°").arg(cycleDeg, 0, 'f', 3));
            });
    connect(&motionService_, &MotionService::stateChanged, this, [this](const QString& state) {
        if (state != QStringLiteral("JOGGING")) {
            jogUiDirection_ = 0;
            updateJogButtonStyle();
        }
    });
    connect(&motionService_, &MotionService::motionFinished, this, [this]() {
        if (!fineHoming_ || fineHomeStage_ != FineHomeStage::MoveToMidpoint) {
            return;
        }
        QString error;
        if (!boardAdapter_.zeroAxis(appConfig_.axisIndex, &error)) {
            finishFineHoming(false, QStringLiteral("精零点置零失败: %1").arg(error));
            return;
        }
        motionService_.refreshPosition();
        finishFineHoming(
            true,
            QStringLiteral("精零点完成: P1=%1, P2=%2, P0=%3")
                .arg(fineHomeFirstEdgePulse_)
                .arg(fineHomeSecondEdgePulse_)
                .arg(fineHomeMidPulse_));
    });

    connect(&projectorService_, &ProjectorService::projectorLog, this,
            [this](const QString& msg) { logService_.info(QStringLiteral("光机"), msg); });
    connect(&projectorService_, &ProjectorService::portStateChanged, this,
            [this](bool opened, const QString& portName) {
                lblProjectorPortState_->setText(
                    opened ? QStringLiteral("光机串口状态: 已连接（%1）").arg(portName)
                           : QStringLiteral("光机串口状态: 未连接"));
                if (opened) {
                    lastProjectorPort_ = portName.trimmed().toUpper();
                }
            });

    auto applyProjectionPreviewPixmap = [this](const QPixmap& pixmap, const QString& fallbackText) {
        if (lblProjectionPreview_ == nullptr) {
            return;
        }
        if (!pixmap.isNull()) {
            lblProjectionPreview_->setText(QString());
            lblProjectionPreview_->setPixmap(
                pixmap.scaled(lblProjectionPreview_->size(), Qt::KeepAspectRatio,
                              Qt::SmoothTransformation));
            return;
        }
        lblProjectionPreview_->setPixmap(QPixmap());
        lblProjectionPreview_->setText(fallbackText);
    };
    auto restoreProjectionPreview = [this, applyProjectionPreviewPixmap]() {
        if (!checkedFrames_.isEmpty()) {
            const QPixmap pixmap(checkedFrames_.first().filePath);
            applyProjectionPreviewPixmap(pixmap, QStringLiteral("实时投影预览"));
        } else {
            applyProjectionPreviewPixmap(QPixmap(), QStringLiteral("实时投影预览"));
        }
        if (lblCurrentFrame_ != nullptr) {
            lblCurrentFrame_->setText(QStringLiteral("当前投影: N/A"));
        }
    };
    auto refreshAlignmentPreview = [this, applyProjectionPreviewPixmap]() {
        applyProjectionPreviewPixmap(projectionWindow_.currentPixmap(), QStringLiteral("实时投影预览"));
        if (lblCurrentFrame_ != nullptr) {
            lblCurrentFrame_->setText(projectionWindow_.currentStatusText());
        }
    };

    connect(&syncPlaybackService_, &SyncPlaybackService::stateChanged, this,
            [this](const QString& state) {
                lblSyncState_->setText(QStringLiteral("同步状态: %1").arg(state));
            });
    connect(&syncPlaybackService_, &SyncPlaybackService::progressChanged, this,
            [this](int loopIndex, int loopTarget, double cycleDeg) {
                const QString targetText = loopTarget <= 0 ? QStringLiteral("∞")
                                                           : QString::number(loopTarget);
                lblSyncProgress_->setText(
                    QStringLiteral("同步进度: %1/%2  当前角度: %3°")
                        .arg(loopIndex)
                        .arg(targetText)
                        .arg(cycleDeg, 0, 'f', 3));
            });
    connect(&syncPlaybackService_, &SyncPlaybackService::frameChanged, this,
            [this](int frameIndex, int loopIndex, const QString& filePath, double angleDeg) {
                lblCurrentFrame_->setText(QStringLiteral("当前投影: 第%1张  圈次%2  角度%3°")
                                              .arg(frameIndex + 1)
                                              .arg(loopIndex + 1)
                                              .arg(angleDeg, 0, 'f', 3));
                projectionWindow_.displayImage(filePath);
                if (lblProjectionPreview_ != nullptr) {
                    QPixmap px(filePath);
                    if (!px.isNull()) {
                        lblProjectionPreview_->setText(QString());
                        lblProjectionPreview_->setPixmap(
                            px.scaled(lblProjectionPreview_->size(), Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation));
                    }
                }
            });
    connect(&syncPlaybackService_, &SyncPlaybackService::syncFinished, this, [this](bool completed) {
        syncElapsedUiTimer_.stop();
        if (syncElapsedTimer_.isValid()) {
            syncElapsedMs_ = syncElapsedTimer_.elapsed();
        }
        updateSyncElapsedLabel(syncElapsedMs_);
        btnSyncStart_->setEnabled(true);
        btnSyncStop_->setEnabled(false);
        projectionWindow_.hideProjection();
        if (completed) {
            shutdownProjector(false);
        }
        updateBoardDependentUiState();
    });
    connect(&syncElapsedUiTimer_, &QTimer::timeout, this, [this]() {
        updateSyncElapsedLabel();
    });

    connect(&cameraService_, &CameraService::cameraLog, this,
            [this](const QString& msg) { logService_.info(QStringLiteral("相机"), msg); });
    auto applyCameraParams = [this](bool silent) {
        if (!cameraService_.isOpen()) {
            return;
        }
        if (spinCameraBrightness_ == nullptr || spinCameraContrast_ == nullptr ||
            spinCameraSaturation_ == nullptr || spinCameraExposure_ == nullptr) {
            return;
        }
        QString error;
        if (!cameraService_.applyImageSettings(spinCameraBrightness_->value(),
                                               spinCameraContrast_->value(),
                                               spinCameraSaturation_->value(),
                                               spinCameraExposure_->value(),
                                               cameraMonochrome_, &error) &&
            !silent) {
            logService_.error(QStringLiteral("相机"), error);
        }
    };
    auto updateCameraModeButton = [this]() {
        if (btnCameraModeToggle_ == nullptr) {
            return;
        }
        if (cameraMonochrome_) {
            btnCameraModeToggle_->setText(QStringLiteral("模式: 黑白"));
            btnCameraModeToggle_->setStyleSheet(
                QStringLiteral("QPushButton{background:#3f3f3f;color:white;border:1px solid #222;}"
                               "QPushButton:pressed{background:#2b2b2b;}"));
        } else {
            btnCameraModeToggle_->setText(QStringLiteral("模式: 彩色"));
            btnCameraModeToggle_->setStyleSheet(
                QStringLiteral("QPushButton{background:#f0f0f0;color:#222;border:1px solid #999;}"
                               "QPushButton:pressed{background:#e2e2e2;}"));
        }
    };
    auto updateRecordButtonStyle = [this](bool recording) {
        if (btnRec_ == nullptr) {
            return;
        }
        btnRec_->setText(QStringLiteral("录像"));
        if (recording) {
            btnRec_->setStyleSheet(
                QStringLiteral("QPushButton{background:#d9363e;color:white;border:1px solid #8f1d22;font-weight:700;}"
                               "QPushButton:pressed{background:#b4242b;}"));
        } else {
            btnRec_->setStyleSheet(
                QStringLiteral("QPushButton{background:#f0f0f0;color:#222;border:1px solid #999;}"
                               "QPushButton:pressed{background:#e2e2e2;}"));
        }
    };
    connect(&cameraService_, &CameraService::cameraListChanged, this,
            [this](const QStringList& names) {
                const QString current = comboCameraList_ != nullptr
                                            ? comboCameraList_->currentText()
                                            : QString();
                if (comboCameraList_ == nullptr) {
                    return;
                }
                comboCameraList_->clear();
                if (names.isEmpty()) {
                    comboCameraList_->addItem(QStringLiteral("未检测到相机"));
                    if (btnCameraOpen_ != nullptr) {
                        btnCameraOpen_->setEnabled(false);
                    }
                } else {
                    comboCameraList_->addItems(names);
                    if (btnCameraOpen_ != nullptr) {
                        btnCameraOpen_->setEnabled(true);
                    }
                    const int idx = names.indexOf(current);
                    if (idx >= 0) {
                        comboCameraList_->setCurrentIndex(idx);
                    }
                }

                if (!cameraAutoOpenEvaluated_) {
                    cameraAutoOpenEvaluated_ = true;
                    if (names.size() == 1 && !cameraService_.isOpen()) {
                        comboCameraList_->setCurrentIndex(0);
                        QString error;
                        if (!cameraService_.openCamera(0, &error)) {
                            logService_.error(QStringLiteral("相机"),
                                              QStringLiteral("启动自动打开相机失败: %1").arg(error));
                        } else {
                            logService_.info(QStringLiteral("相机"),
                                             QStringLiteral("启动自动打开相机成功。"));
                        }
                    } else if (names.size() != 1) {
                        logService_.info(QStringLiteral("相机"),
                                         QStringLiteral("启动自动打开相机已跳过: 当前检测到%1个设备。")
                                             .arg(names.size()));
                    }
                }
            });
    connect(&cameraService_, &CameraService::cameraStateChanged, this,
            [this, applyCameraParams, updateRecordButtonStyle, updateCameraModeButton](bool opened, const QString& name) {
                if (lblCameraState_ != nullptr) {
                    lblCameraState_->setText(
                        opened ? QStringLiteral("相机状态: 已打开(%1)").arg(name)
                               : QStringLiteral("相机状态: 未打开"));
                }
                if (btnCameraOpen_ != nullptr) {
                    btnCameraOpen_->setText(opened ? QStringLiteral("关闭相机")
                                                  : QStringLiteral("打开相机"));
                }
                if (btnSnap_ != nullptr) {
                    btnSnap_->setEnabled(opened);
                }
                if (btnRec_ != nullptr) {
                    btnRec_->setEnabled(opened);
                }
                if (btnCameraModeToggle_ != nullptr) {
                    btnCameraModeToggle_->setEnabled(opened);
                }
                if (spinCameraBrightness_ != nullptr) {
                    spinCameraBrightness_->setEnabled(opened);
                }
                if (spinCameraContrast_ != nullptr) {
                    spinCameraContrast_->setEnabled(opened);
                }
                if (spinCameraSaturation_ != nullptr) {
                    spinCameraSaturation_->setEnabled(opened);
                }
                if (spinCameraExposure_ != nullptr) {
                    spinCameraExposure_->setEnabled(opened);
                }
                if (opened) {
                    cameraMonochrome_ = false;
                }
                updateRecordButtonStyle(false);
                updateCameraModeButton();
                if (opened) {
                    applyCameraParams(true);
                } else if (lblCameraRecordTime_ != nullptr) {
                    lblCameraRecordTime_->setText(QStringLiteral("录像时长: 00:00:00"));
                }
            });
    connect(&cameraService_, &CameraService::recordingStateChanged, this,
            [this, updateRecordButtonStyle](bool recording, const QString& filePath) {
                updateRecordButtonStyle(recording);
                if (recording) {
                    logService_.info(QStringLiteral("相机"),
                                     QStringLiteral("录像中: %1")
                                         .arg(QDir::toNativeSeparators(filePath)));
                } else {
                    logService_.info(QStringLiteral("相机"), QStringLiteral("录像已停止。"));
                }
            });
    connect(&cameraService_, &CameraService::recordingDurationChanged, this,
            [this](qint64 msec) {
                if (lblCameraRecordTime_ == nullptr) {
                    return;
                }
                const qint64 sec = qMax<qint64>(0, msec / 1000);
                const qint64 hour = sec / 3600;
                const qint64 minute = (sec % 3600) / 60;
                const qint64 second = sec % 60;
                lblCameraRecordTime_->setText(
                    QStringLiteral("录像时长: %1:%2:%3")
                        .arg(hour, 2, 10, QLatin1Char('0'))
                        .arg(minute, 2, 10, QLatin1Char('0'))
                        .arg(second, 2, 10, QLatin1Char('0')));
            });

    connect(btnConnectCard_, &QPushButton::clicked, this, [this]() {
        if (boardAdapter_.isConnected()) {
            boardAdapter_.disconnectBoard();
        } else {
            if (!autoConnectBoard()) {
                logService_.error(QStringLiteral("板卡"), QStringLiteral("板卡自动连接失败。"));
            }
        }
    });
    connect(btnBoardRefresh_, &QPushButton::clicked, this, [this]() { refreshBoardPorts(); });

    if (btnCameraRefresh_ != nullptr) {
        connect(btnCameraRefresh_, &QPushButton::clicked, this, [this]() {
            cameraService_.refreshCameras();
        });
    }
    if (btnCameraOpen_ != nullptr) {
        connect(btnCameraOpen_, &QPushButton::clicked, this, [this]() {
            if (cameraService_.isOpen()) {
                cameraService_.closeCamera();
                return;
            }
            cameraService_.refreshCameras();
            if (comboCameraList_ == nullptr || comboCameraList_->count() <= 0 ||
                comboCameraList_->currentText() == QStringLiteral("未检测到相机")) {
                logService_.warn(QStringLiteral("相机"), QStringLiteral("未检测到可用相机。"));
                return;
            }
            const int index = comboCameraList_->currentIndex();
            QString error;
            if (!cameraService_.openCamera(index, &error)) {
                logService_.error(QStringLiteral("相机"), error);
            }
        });
    }
    if (btnSnap_ != nullptr) {
        connect(btnSnap_, &QPushButton::clicked, this, [this]() {
            QString savedPath;
            QString error;
            if (!cameraService_.captureImage(&savedPath, &error)) {
                logService_.error(QStringLiteral("相机"), error);
                return;
            }
            logService_.info(QStringLiteral("相机"),
                             QStringLiteral("已触发拍照: %1").arg(savedPath));
        });
    }
    if (btnRec_ != nullptr) {
        connect(btnRec_, &QPushButton::clicked, this, [this]() {
            if (!cameraService_.isOpen()) {
                logService_.warn(QStringLiteral("相机"), QStringLiteral("相机未打开，无法录像。"));
                return;
            }
            if (cameraService_.isRecording()) {
                cameraService_.stopRecording();
                return;
            }
            btnRec_->setEnabled(false);
            btnRec_->setText(QStringLiteral("启动中..."));
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 20);
            QString savedPath;
            QString error;
            if (!cameraService_.startRecording(&savedPath, &error)) {
                logService_.error(QStringLiteral("相机"), error);
                btnRec_->setEnabled(true);
                btnRec_->setText(QStringLiteral("录像"));
                btnRec_->setStyleSheet(
                    QStringLiteral("QPushButton{background:#f0f0f0;color:#222;border:1px solid #999;}"
                                   "QPushButton:pressed{background:#e2e2e2;}"));
            } else {
                btnRec_->setEnabled(true);
            }
        });
    }
    if (btnCameraModeToggle_ != nullptr) {
        connect(btnCameraModeToggle_, &QPushButton::clicked, this,
                [this, applyCameraParams, updateCameraModeButton]() {
                    if (!cameraService_.isOpen()) {
                        return;
                    }
                    cameraMonochrome_ = !cameraMonochrome_;
                    updateCameraModeButton();
                    applyCameraParams(true);
                });
    }
    auto bindAutoApplySpin = [this](QDoubleSpinBox* spin) {
        if (spin == nullptr) {
            return;
        }
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                [this](double) {
                    if (!cameraService_.isOpen()) {
                        return;
                    }
                    QString error;
                    cameraService_.applyImageSettings(
                        spinCameraBrightness_ != nullptr ? spinCameraBrightness_->value() : 0.0,
                        spinCameraContrast_ != nullptr ? spinCameraContrast_->value() : 0.0,
                        spinCameraSaturation_ != nullptr ? spinCameraSaturation_->value() : 0.0,
                        spinCameraExposure_ != nullptr ? spinCameraExposure_->value() : 0.0,
                        cameraMonochrome_, &error);
                });
    };
    bindAutoApplySpin(spinCameraBrightness_);
    bindAutoApplySpin(spinCameraContrast_);
    bindAutoApplySpin(spinCameraSaturation_);
    bindAutoApplySpin(spinCameraExposure_);
    connect(&boardPortScanTimer_, &QTimer::timeout, this, [this]() {
        if (!boardAdapter_.isConnected()) {
            refreshBoardPorts();
        }
    });
    connect(&coarseHomeTimer_, &QTimer::timeout, this, [this]() {
        if (!coarseHoming_) {
            return;
        }
        coarseHomeElapsedMs_ += coarseHomeTimer_.interval();

        if (coarseHomeStage_ == CoarseHomeStage::StopSettle) {
            coarseHomeConfirmRemainMs_ -= coarseHomeTimer_.interval();
            if (coarseHomeConfirmRemainMs_ > 0) {
                return;
            }
            motionService_.refreshPosition();
            coarseHomeConfirmPulse_ = motionService_.currentPulse();
            logService_.info(
                QStringLiteral("回零"),
                QStringLiteral("粗零点停稳确认: 触发脉冲=%1, 停稳脉冲=%2, 差值=%3")
                    .arg(coarseHomeFirstEdgePulse_)
                    .arg(coarseHomeConfirmPulse_)
                    .arg(coarseHomeConfirmPulse_ - coarseHomeFirstEdgePulse_));
            finishCoarseHoming(true, QStringLiteral("粗零点完成: 单边沿触发即停。"));
            return;
        }

        if (coarseHomeStage_ == CoarseHomeStage::SeekEdge) {
            const qint64 traveledPulse =
                qAbs(motionService_.currentPulse() - coarseHomeStartPulse_);
            const qint64 maxPulse = static_cast<qint64>(
                qRound64(appConfig_.pulsePerDeg * 360.0 * appConfig_.coarseHomeMaxTurns));
            if (traveledPulse >= maxPulse) {
                finishCoarseHoming(false,
                                  QStringLiteral("粗零点失败: 超过%1圈未触发。")
                                      .arg(appConfig_.coarseHomeMaxTurns, 0, 'f', 3));
                return;
            }
        }

        if (coarseHomeElapsedMs_ >= appConfig_.homeTimeoutMs) {
            finishCoarseHoming(false, QStringLiteral("粗零点失败: 超时未完成边沿定位。"));
        }
    });
    connect(&fineHomeTimer_, &QTimer::timeout, this, [this]() {
        if (!fineHoming_) {
            return;
        }
        fineHomeElapsedMs_ += fineHomeTimer_.interval();
        if (fineHomeStage_ == FineHomeStage::SeekFirstEdge ||
            fineHomeStage_ == FineHomeStage::SeekSecondEdge) {
            const qint64 traveledPulse =
                qAbs(motionService_.currentPulse() - fineHomeStartPulse_);
            const qint64 maxPulse = static_cast<qint64>(
                qRound64(appConfig_.fineHomeMaxTravelDeg * appConfig_.pulsePerDeg));
            if (traveledPulse >= maxPulse) {
                finishFineHoming(
                    false,
                    QStringLiteral("精零点失败: 超过%1°未完成 DI1 打开/关闭采集。")
                        .arg(appConfig_.fineHomeMaxTravelDeg, 0, 'f', 3));
                return;
            }
        }

        if (fineHomeElapsedMs_ >= appConfig_.fineHomeTimeoutMs) {
            const QString detail =
                fineHomeStage_ == FineHomeStage::SeekFirstEdge
                    ? QStringLiteral("未检测到 DI1 打开。")
                    : (fineHomeStage_ == FineHomeStage::SeekSecondEdge
                           ? QStringLiteral("已检测到 DI1 打开，但未检测到 DI1 关闭。")
                           : QStringLiteral("回中定位超时。"));
            finishFineHoming(false, QStringLiteral("精零点失败: %1").arg(detail));
        }
    });
    boardPortScanTimer_.start();

    auto* btnJogPlus = btnJogPlusMotion_;
    auto* btnJogMinus = btnJogMinusMotion_;
    auto* btnStop = btnStopMotion_;
    auto* btnEStop = btnEStopMotion_;
    auto* btnMoveAbs = btnMoveAbsMotion_;
    auto* btnMoveRel = btnMoveRelMotion_;

    connect(btnJogPlus, &QPushButton::clicked, this, [this]() {
        if (!boardAdapter_.isConnected()) {
            logService_.warn(QStringLiteral("运动"), QStringLiteral("板卡未连接，无法点动。"));
            return;
        }
        if (safetyTripped_) {
            logService_.warn(QStringLiteral("安全"), QStringLiteral("当前处于 ALARM，禁止点动。"));
            return;
        }
        if (motionService_.stateText() == QStringLiteral("JOGGING") && jogUiDirection_ == +1) {
            motionService_.stop(false);
            jogUiDirection_ = 0;
            updateJogButtonStyle();
            return;
        }
        double speed = 0.0;
        if (!resolveJogSpeed(speed)) {
            return;
        }
        if (motionService_.stateText() == QStringLiteral("JOGGING")) {
            motionService_.stop(false);
        }
        if (motionService_.jogStart(+1, speed)) {
            jogUiDirection_ = +1;
        } else {
            jogUiDirection_ = 0;
        }
        updateJogButtonStyle();
    });
    connect(btnJogMinus, &QPushButton::clicked, this, [this]() {
        if (!boardAdapter_.isConnected()) {
            logService_.warn(QStringLiteral("运动"), QStringLiteral("板卡未连接，无法点动。"));
            return;
        }
        if (safetyTripped_) {
            logService_.warn(QStringLiteral("安全"), QStringLiteral("当前处于 ALARM，禁止点动。"));
            return;
        }
        if (motionService_.stateText() == QStringLiteral("JOGGING") && jogUiDirection_ == -1) {
            motionService_.stop(false);
            jogUiDirection_ = 0;
            updateJogButtonStyle();
            return;
        }
        double speed = 0.0;
        if (!resolveJogSpeed(speed)) {
            return;
        }
        if (motionService_.stateText() == QStringLiteral("JOGGING")) {
            motionService_.stop(false);
        }
        if (motionService_.jogStart(-1, speed)) {
            jogUiDirection_ = -1;
        } else {
            jogUiDirection_ = 0;
        }
        updateJogButtonStyle();
    });
    connect(btnStop, &QPushButton::clicked, this, [this]() {
        motionService_.stop(false);
        jogUiDirection_ = 0;
        updateJogButtonStyle();
        if (coarseHoming_) {
            finishCoarseHoming(false, QStringLiteral("粗零点已手动停止。"));
        }
    });
    connect(btnEStop, &QPushButton::clicked, this, [this]() {
        motionService_.stop(true);
        jogUiDirection_ = 0;
        updateJogButtonStyle();
        if (coarseHoming_) {
            finishCoarseHoming(false, QStringLiteral("粗零点已急停中断。"));
        }
    });

    connect(btnMoveAbs, &QPushButton::clicked, this, [this]() {
        if (!boardAdapter_.isConnected()) {
            logService_.warn(QStringLiteral("运动"), QStringLiteral("板卡未连接，无法运动。"));
            return;
        }
        if (safetyTripped_) {
            logService_.warn(QStringLiteral("安全"), QStringLiteral("当前处于 ALARM，禁止运动。"));
            return;
        }
        motionService_.moveToAbsDeg(editTargetAbsDeg_ != nullptr ? editTargetAbsDeg_->value() : 0.0);
    });

    connect(btnMoveRel, &QPushButton::clicked, this, [this]() {
        if (!boardAdapter_.isConnected()) {
            logService_.warn(QStringLiteral("运动"), QStringLiteral("板卡未连接，无法运动。"));
            return;
        }
        if (safetyTripped_) {
            logService_.warn(QStringLiteral("安全"), QStringLiteral("当前处于 ALARM，禁止运动。"));
            return;
        }
        motionService_.moveByRelDeg(editTargetRelDeg_ != nullptr ? editTargetRelDeg_->value() : 0.0);
    });

    connect(btnCoarseCheck_, &QPushButton::clicked, this, [this]() {
        startCoarseHoming();
    });
    connect(btnFineHome_, &QPushButton::clicked, this, [this]() {
        startFineHoming();
    });

    connect(btnConfirmZReady_, &QPushButton::clicked, this, [this]() {
        if (!laserStableForZConfirm()) {
            logService_.warn(QStringLiteral("Z确认"),
                             QStringLiteral("当前对射状态未稳定，无法确认Z到位。"));
            return;
        }
        const QString now = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        lblZConfirmTime_->setText(QStringLiteral("Z确认时间: %1").arg(now));
        logService_.info(
            QStringLiteral("Z确认"),
            QStringLiteral("操作员确认Z到位，对射状态稳定时间达到 %1 ms。").arg(appConfig_.laserStableMs));
    });

    connect(btnProjectorRefresh_, &QPushButton::clicked, this, [this]() {
        refreshProjectorPorts();
    });
    connect(btnProjectorOpen_, &QPushButton::clicked, this, [this]() {
        if (!autoConnectProjector()) {
            logService_.error(QStringLiteral("光机"), QStringLiteral("光机自动连接失败。"));
        }
    });
    connect(btnProjectorClose_, &QPushButton::clicked, this, [this]() {
        projectorService_.closePort();
    });
    connect(btnProjectorPowerOn_, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!projectorService_.setPower(true, &error)) {
            logService_.error(QStringLiteral("光机"), error);
        }
    });
    connect(btnProjectorPowerOff_, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!projectorService_.setPower(false, &error)) {
            logService_.error(QStringLiteral("光机"), error);
        }
    });
    connect(btnLedOn_, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!projectorService_.setLed(true, &error)) {
            logService_.error(QStringLiteral("光机"), error);
        }
    });
    connect(btnLedOff_, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!projectorService_.setLed(false, &error)) {
            logService_.error(QStringLiteral("光机"), error);
        }
    });
    connect(btnApplyLedPercent_, &QPushButton::clicked, this, [this]() {
        int percent = editLedPercent_ != nullptr ? editLedPercent_->value() : 0;
        QString error;
        if (!projectorService_.setBrightnessPercent(percent, &error)) {
            logService_.error(QStringLiteral("光机"), error);
        }
    });
    connect(btnAlignSiemens_, &QPushButton::clicked, this, [this, refreshAlignmentPreview]() {
        if (syncPlaybackService_.isRunning()) {
            logService_.warn(QStringLiteral("光机"), QStringLiteral("同步运行中，无法进入对准模式。"));
            return;
        }
        projectionWindow_.showSiemensStarAlignment();
        projectionWindow_.showForProjection();
        refreshAlignmentPreview();
        logService_.info(QStringLiteral("光机"),
                         QStringLiteral("进入光机对准模式: 当前为西门子星。方向键可调整，按 Esc 切到三线条，再按 Esc 结束。"));
    });

    connect(btnBrowseImageFolder_, &QPushButton::clicked, this, [this]() {
        const QString folder = QFileDialog::getExistingDirectory(
            this, QStringLiteral("选择图片文件夹"), editImageFolder_->text());
        if (!folder.isEmpty()) {
            editImageFolder_->setText(folder);
            runImageFolderCheck();
        }
    });

    connect(btnCheckImageFolder_, &QPushButton::clicked, this, [this]() { runImageFolderCheck(); });
    connect(editImageFolder_, &QLineEdit::editingFinished, this, [this]() { runImageFolderCheck(); });

    connect(btnSyncStart_, &QPushButton::clicked, this, [this]() {
        if (!boardAdapter_.isConnected()) {
            logService_.warn(QStringLiteral("同步"), QStringLiteral("板卡未连接，无法启动同步。"));
            return;
        }
        if (coarseHoming_ || fineHoming_) {
            logService_.warn(QStringLiteral("同步"), QStringLiteral("回零进行中，无法启动同步。"));
            return;
        }
        if (safetyTripped_) {
            logService_.warn(QStringLiteral("安全"), QStringLiteral("当前处于 ALARM，禁止启动同步。"));
            return;
        }
        if (!runImageFolderCheck()) {
            const QString sequenceMessage =
                lblImageCheckResult_ != nullptr ? lblImageCheckResult_->text()
                                                : QStringLiteral("图片序列无效。");
            logService_.warn(QStringLiteral("同步"),
                             QStringLiteral("同步启动失败: %1").arg(sequenceMessage));
            return;
        }

        const double roundSec = editRoundTimeSec_ != nullptr ? editRoundTimeSec_->value() : 0.0;
        if (roundSec <= 0.0) {
            logService_.warn(QStringLiteral("同步"), QStringLiteral("同步启动失败: 一圈时间无效。"));
            return;
        }

        const int brightnessPercent = editLedPercent_ != nullptr ? editLedPercent_->value() : 0;
        const QString loopText =
            (chkInfiniteLoop_ != nullptr && chkInfiniteLoop_->isChecked())
                ? QStringLiteral("无限循环")
                : QStringLiteral("%1 次").arg(spinLoopCount_ != nullptr ? spinLoopCount_->value() : 0);
        const QString folderText =
            (editImageFolder_ != nullptr && !editImageFolder_->text().trimmed().isEmpty())
                ? QDir::toNativeSeparators(editImageFolder_->text().trimmed())
                : QStringLiteral("未设置");
        const QString roundTimeText = QStringLiteral("%1 s").arg(roundSec, 0, 'f', 3);
        const auto confirm = QMessageBox::question(
            this,
            QStringLiteral("开始同步确认"),
            QStringLiteral("请确认以下信息：\n\n"
                           "亮度: %1%%\n"
                           "循环: %2\n"
                           "一圈时间: %3\n"
                           "投影序列文件夹: %4\n\n"
                           "是否已确认零点以及 Z 到位？")
                .arg(brightnessPercent)
                .arg(loopText)
                .arg(roundTimeText)
                .arg(folderText),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Cancel);
        if (confirm != QMessageBox::Ok) {
            return;
        }

        if (!projectorService_.isPortOpen() && !autoConnectProjector()) {
            logService_.error(QStringLiteral("光机"), QStringLiteral("光机自动连接失败，无法启动同步。"));
            return;
        }

        QString error;
        if (!projectorService_.setPower(true, &error)) {
            logService_.error(QStringLiteral("光机"),
                              QStringLiteral("启动同步前打开光机失败: %1").arg(error));
            return;
        }
        error.clear();
        if (!projectorService_.setLed(true, &error)) {
            logService_.error(QStringLiteral("光机"),
                              QStringLiteral("启动同步前打开LED失败: %1").arg(error));
            return;
        }

        syncPlaybackService_.setFrames(checkedFrames_);
        if (!syncPlaybackService_.start(roundSec, spinLoopCount_->value(),
                                        chkInfiniteLoop_->isChecked(), &error)) {
            logService_.error(QStringLiteral("同步"), error);
            return;
        }

        syncElapsedMs_ = 0;
        syncElapsedTimer_.start();
        updateSyncElapsedLabel(0);
        syncElapsedUiTimer_.start();
        projectionWindow_.clearAlignment();
        projectionWindow_.showForProjection();
        btnSyncStart_->setEnabled(false);
        btnSyncStop_->setEnabled(true);
        updateBoardDependentUiState();
    });

    connect(btnSyncStop_, &QPushButton::clicked, this, [this]() {
        stopSyncWorkflow(false);
    });
    connect(chkInfiniteLoop_, &QCheckBox::toggled, this,
            [this](bool checked) { spinLoopCount_->setEnabled(!checked); });

    auto* escStopShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    escStopShortcut->setContext(Qt::ApplicationShortcut);
    connect(escStopShortcut, &QShortcut::activated, this,
            [this, restoreProjectionPreview, refreshAlignmentPreview]() {
        if (syncPlaybackService_.isRunning()) {
            stopSyncWorkflow(true);
            logService_.warn(QStringLiteral("同步"),
                             QStringLiteral("已通过 Esc 停止同步，光机已关机并关闭LED。"));
            return;
        }
        if (projectionWindow_.isAlignmentActive()) {
            if (projectionWindow_.contentMode() == ProjectionWindow::ContentMode::SiemensStar) {
                projectionWindow_.showThreeLineAlignment();
                refreshAlignmentPreview();
                logService_.info(QStringLiteral("光机"),
                                 QStringLiteral("已切换到三线条对准模式。方向键: 上下调整线宽，左外右内调整线距，再按 Esc 结束。"));
            } else {
                projectionWindow_.clearAlignment();
                projectionWindow_.hideProjection();
                restoreProjectionPreview();
                logService_.info(QStringLiteral("光机"), QStringLiteral("已通过 Esc 退出对准模式。"));
            }
            return;
        }
        if (coarseHoming_) {
            finishCoarseHoming(false, QStringLiteral("粗零点已通过 Esc 停止。"));
            return;
        }
        if (fineHoming_) {
            finishFineHoming(false, QStringLiteral("精零点已通过 Esc 停止。"));
            return;
        }
        if (motionService_.stateText() != QStringLiteral("READY")) {
            motionService_.stop(true);
            jogUiDirection_ = 0;
            updateJogButtonStyle();
            logService_.warn(QStringLiteral("运动"), QStringLiteral("已通过 Esc 停止当前运动。"));
        }
    });
    auto bindAlignmentShortcut = [this, refreshAlignmentPreview](int key) {
        auto* shortcut = new QShortcut(QKeySequence(key), this);
        shortcut->setContext(Qt::ApplicationShortcut);
        connect(shortcut, &QShortcut::activated, this, [this, key, refreshAlignmentPreview]() {
            if (!projectionWindow_.isAlignmentActive()) {
                return;
            }
            if (projectionWindow_.handleAlignmentKey(key)) {
                refreshAlignmentPreview();
            }
        });
    };
    bindAlignmentShortcut(Qt::Key_Up);
    bindAlignmentShortcut(Qt::Key_Down);
    bindAlignmentShortcut(Qt::Key_Left);
    bindAlignmentShortcut(Qt::Key_Right);

    cameraService_.refreshCameras();

    QTimer::singleShot(300, this, [this]() {
        refreshBoardPorts();
        refreshProjectorPorts();

        if (!boardAdapter_.isConnected() && !autoConnectBoard()) {
            logService_.info(QStringLiteral("板卡"),
                             QStringLiteral("启动自动连接未成功，可手动点击“连接板卡”重试。"));
        }

        if (!projectorService_.isPortOpen() && !autoConnectProjector()) {
            logService_.info(QStringLiteral("光机"),
                             QStringLiteral("启动自动连接未成功，可手动点击“连接光机”重试。"));
        }
    });
}

void MainWindow::refreshProjectorPorts() {
    const QString current = comboProjectorPort_->currentText();
    comboProjectorPort_->clear();

    auto normalizeCom = [](QString port) {
        port = port.trimmed().toUpper();
        if (port.isEmpty()) {
            return port;
        }
        bool ok = false;
        const int number = port.toInt(&ok);
        if (ok && number > 0) {
            return QStringLiteral("COM%1").arg(number);
        }
        if (!port.startsWith(QStringLiteral("COM"))) {
            return QStringLiteral("COM%1").arg(port);
        }
        return port;
    };

    const QString boardPort =
        boardAdapter_.isConnected() ? normalizeCom(boardAdapter_.currentPort()) : QString();

    QStringList mergedPorts;

    QStringList normalizedScannedPorts;
    const QStringList scannedPorts = projectorService_.availablePorts();
    for (const QString& port : scannedPorts) {
        const QString normalized = normalizeCom(port);
        if (!boardPort.isEmpty() && normalized == boardPort) {
            continue;
        }
        if (!normalized.isEmpty() && !mergedPorts.contains(normalized)) {
            mergedPorts.push_back(normalized);
        }
        if (!normalized.isEmpty() && !normalizedScannedPorts.contains(normalized)) {
            normalizedScannedPorts.push_back(normalized);
        }
    }

    const QString normalizedCurrent = normalizeCom(current);
    if (!normalizedCurrent.isEmpty() &&
        (boardPort.isEmpty() || normalizedCurrent != boardPort) &&
        !mergedPorts.contains(normalizedCurrent)) {
        mergedPorts.push_back(normalizedCurrent);
    }

    if (mergedPorts.isEmpty()) {
        comboProjectorPort_->addItem(QStringLiteral("无可用串口"));
        btnProjectorOpen_->setEnabled(false);
        return;
    }

    comboProjectorPort_->addItems(mergedPorts);
    btnProjectorOpen_->setEnabled(true);

    if (!normalizedCurrent.isEmpty()) {
        const int index = mergedPorts.indexOf(normalizedCurrent);
        if (index >= 0) {
            comboProjectorPort_->setCurrentIndex(index);
            return;
        }
    }

    const QString openedPort = normalizeCom(projectorService_.currentPortName());
    if (!openedPort.isEmpty() && openedPort != QStringLiteral("N/A")) {
        const int index = mergedPorts.indexOf(openedPort);
        if (index >= 0) {
            comboProjectorPort_->setCurrentIndex(index);
            return;
        }
    }

    if (!lastProjectorPort_.isEmpty()) {
        const int index = mergedPorts.indexOf(normalizeCom(lastProjectorPort_));
        if (index >= 0) {
            comboProjectorPort_->setCurrentIndex(index);
            return;
        }
    }

    if (!normalizedScannedPorts.isEmpty()) {
        const int index = mergedPorts.indexOf(normalizedScannedPorts.first());
        if (index >= 0) {
            comboProjectorPort_->setCurrentIndex(index);
            return;
        }
    }

    comboProjectorPort_->setCurrentIndex(0);
}

void MainWindow::refreshBoardPorts() {
    const QString current = comboBoardPort_->currentText();
    comboBoardPort_->clear();

    auto normalizeCom = [](QString port) {
        port = port.trimmed().toUpper();
        if (port.isEmpty()) {
            return port;
        }
        bool ok = false;
        const int number = port.toInt(&ok);
        if (ok && number > 0) {
            return QStringLiteral("COM%1").arg(number);
        }
        if (!port.startsWith(QStringLiteral("COM"))) {
            return QStringLiteral("COM%1").arg(port);
        }
        return port;
    };

    const QString placeholderText = QStringLiteral("无可用串口");
    QStringList mergedPorts;
    QStringList normalizedScannedPorts;
    const QStringList scannedPorts = boardAdapter_.availablePorts();
    for (const QString& port : scannedPorts) {
        const QString normalized = normalizeCom(port);
        if (!normalized.isEmpty() && !mergedPorts.contains(normalized)) {
            mergedPorts.push_back(normalized);
        }
        if (!normalized.isEmpty() && !normalizedScannedPorts.contains(normalized)) {
            normalizedScannedPorts.push_back(normalized);
        }
    }

    const QString normalizedCurrent =
        current == placeholderText ? QString() : normalizeCom(current);
    if (!normalizedCurrent.isEmpty() && !mergedPorts.contains(normalizedCurrent)) {
        mergedPorts.push_back(normalizedCurrent);
    }

    if (mergedPorts.isEmpty()) {
        comboBoardPort_->addItem(placeholderText);
        btnConnectCard_->setEnabled(boardAdapter_.isConnected());
        return;
    }

    comboBoardPort_->addItems(mergedPorts);
    btnConnectCard_->setEnabled(true);

    if (!normalizedCurrent.isEmpty()) {
        const int index = mergedPorts.indexOf(normalizedCurrent);
        if (index >= 0) {
            comboBoardPort_->setCurrentIndex(index);
            return;
        }
    }

    const QString openedPort = normalizeCom(boardAdapter_.currentPort());
    if (!openedPort.isEmpty()) {
        const int index = mergedPorts.indexOf(openedPort);
        if (index >= 0) {
            comboBoardPort_->setCurrentIndex(index);
            return;
        }
    }

    if (!lastBoardPort_.isEmpty()) {
        const int index = mergedPorts.indexOf(normalizeCom(lastBoardPort_));
        if (index >= 0) {
            comboBoardPort_->setCurrentIndex(index);
            return;
        }
    }

    if (!normalizedScannedPorts.isEmpty()) {
        const int index = mergedPorts.indexOf(normalizedScannedPorts.first());
        if (index >= 0) {
            comboBoardPort_->setCurrentIndex(index);
            return;
        }
    }

    comboBoardPort_->setCurrentIndex(0);
}

bool MainWindow::runImageFolderCheck() {
    auto setImageCheckBadge = [this](const QString& text, bool ok) {
        lblImageCheckResult_->setText(text);
        lblImageCheckResult_->setStyleSheet(
            ok
                ? QStringLiteral("QLabel{background:#ecfdf3;border:1px solid #a6f4c5;color:#067647;border-radius:6px;padding:4px 8px;font-weight:600;}")
                : QStringLiteral("QLabel{background:#fef3f2;border:1px solid #f5c2c7;color:#b42318;border-radius:6px;padding:4px 8px;font-weight:600;}"));
    };

    const QString folderPath = editImageFolder_->text().trimmed();
    if (folderPath.isEmpty()) {
        checkedFrames_.clear();
        setImageCheckBadge(QStringLiteral("序列状态: 未检查"), false);
        if (lblProjectionPreview_ != nullptr) {
            lblProjectionPreview_->setText(QStringLiteral("实时投影预览"));
            lblProjectionPreview_->setPixmap(QPixmap());
        }
        return false;
    }

    const SequenceCheckResult result = imageSequenceService_.checkFolder(folderPath);
    if (result.ok) {
        setImageCheckBadge(result.message, true);
        checkedFrames_ = result.frames;
        logService_.info(QStringLiteral("序列"), result.message);
        if (!checkedFrames_.isEmpty() && lblProjectionPreview_ != nullptr) {
            QPixmap px(checkedFrames_.first().filePath);
            if (!px.isNull()) {
                lblProjectionPreview_->setText(QString());
                lblProjectionPreview_->setPixmap(
                    px.scaled(lblProjectionPreview_->size(), Qt::KeepAspectRatio,
                              Qt::SmoothTransformation));
            } else {
                lblProjectionPreview_->setText(QStringLiteral("实时投影预览"));
            }
        }
    } else {
        setImageCheckBadge(QStringLiteral("序列异常: %1").arg(result.message), false);
        checkedFrames_.clear();
        if (lblProjectionPreview_ != nullptr) {
            lblProjectionPreview_->setText(QStringLiteral("实时投影预览"));
            lblProjectionPreview_->setPixmap(QPixmap());
        }
        logService_.warn(QStringLiteral("序列"), result.message);
    }
    return result.ok;
}

bool MainWindow::startCoarseHoming() {
    if (safetyTripped_) {
        logService_.warn(QStringLiteral("安全"), QStringLiteral("当前处于 ALARM，禁止回零。"));
        return false;
    }
    if (syncPlaybackService_.isRunning()) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("同步运行中，禁止执行粗零点。"));
        return false;
    }
    if (coarseHoming_) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("粗零点流程已在运行。"));
        return false;
    }
    if (fineHoming_) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("精零点流程已在运行。"));
        return false;
    }
    if (!boardAdapter_.isConnected()) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("请先连接板卡后再执行粗零点。"));
        return false;
    }
    const bool initialDi = boardAdapter_.diBit(appConfig_.coarseHomeDiIndex);
    double jogSpeedPulseMs = 0.0;
    if (!resolveJogSpeed(jogSpeedPulseMs)) {
        return false;
    }
    coarseHomeJogSpeedPulseMs_ = jogSpeedPulseMs;
    boardAdapter_.setPollIntervalMs(5);
    coarseHomeTimer_.setInterval(5);
    if (!motionService_.jogStart(appConfig_.coarseHomeDirection, coarseHomeJogSpeedPulseMs_)) {
        lblHomeResult_->setText(QStringLiteral("回零结果: 粗零点启动失败。"));
        logService_.error(QStringLiteral("回零"), QStringLiteral("粗零点启动失败: 无法进入点动。"));
        return false;
    }

    coarseHoming_ = true;
    coarseHomeElapsedMs_ = 0;
    coarseHomeConfirmRemainMs_ = 0;
    coarseHomeStage_ = CoarseHomeStage::SeekEdge;
    motionService_.refreshPosition();
    coarseHomeStartPulse_ = motionService_.currentPulse();
    coarseHomeInitialDi_ = initialDi;
    coarseHomeFirstEdgePulse_ = 0;
    coarseHomeConfirmPulse_ = 0;
    coarseHomeTimer_.start();
    btnCoarseCheck_->setEnabled(false);
    btnFineHome_->setEnabled(false);
    updateBoardDependentUiState();
    lblHomeResult_->setText(QStringLiteral("回零结果: 粗零点进行中..."));
    lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
    const qint64 maxPulse = static_cast<qint64>(
        qRound64(appConfig_.pulsePerDeg * 360.0 * appConfig_.coarseHomeMaxTurns));
    logService_.info(QStringLiteral("回零"),
                     QStringLiteral("粗零点开始: 单边沿触发法，DI%1，方向%2，速度%3 °/s，初始DI=%4。")
                         .arg(appConfig_.coarseHomeDiIndex)
                         .arg(appConfig_.coarseHomeDirection > 0 ? QStringLiteral("+") : QStringLiteral("-"))
                         .arg(coarseHomeJogSpeedPulseMs_ * 1000.0 / appConfig_.pulsePerDeg, 0, 'f', 3)
                         .arg(coarseHomeInitialDi_ ? QStringLiteral("ON") : QStringLiteral("OFF")));
    logService_.info(
        QStringLiteral("回零"),
        QStringLiteral("粗零点限制: 最大%1圈，最大脉冲=%2，DI轮询间隔=5ms。")
            .arg(appConfig_.coarseHomeMaxTurns, 0, 'f', 3)
            .arg(maxPulse));
    return true;
}

bool MainWindow::startFineHoming() {
    if (safetyTripped_) {
        logService_.warn(QStringLiteral("安全"), QStringLiteral("当前处于 ALARM，禁止精零点。"));
        return false;
    }
    if (syncPlaybackService_.isRunning()) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("同步运行中，禁止执行精零点。"));
        return false;
    }
    if (coarseHoming_) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("粗零点流程进行中，无法启动精零点。"));
        return false;
    }
    if (fineHoming_) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("精零点流程已在运行。"));
        return false;
    }
    if (!boardAdapter_.isConnected()) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("请先连接板卡后再执行精零点。"));
        return false;
    }
    if (motionService_.stateText() != QStringLiteral("READY")) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("当前运动未停止，无法执行精零点。"));
        return false;
    }

    double jogSpeedPulseMs = 0.0;
    if (!resolveJogSpeed(jogSpeedPulseMs)) {
        return false;
    }

    fineHomeJogSpeedPulseMs_ = qMax(0.001, jogSpeedPulseMs * kFineHomeJogSpeedScale);
    motionService_.refreshPosition();
    fineHomeStartPulse_ = motionService_.currentPulse();
    fineHomeLastLaserState_ = laserTriggeredFromRaw(boardAdapter_.diRaw());
    if (fineHomeLastLaserState_) {
        QMessageBox::warning(this,
                             QStringLiteral("精零点不可用"),
                             QStringLiteral("当前 DI1 已触发，无法执行精零点。\n请先点动-调整位置。"));
        logService_.warn(QStringLiteral("回零"),
                         QStringLiteral("精零点启动被拒绝: 当前 DI1 已触发，请先点动-调整位置。"));
        return false;
    }
    fineHomeFirstEdgePulse_ = 0;
    fineHomeSecondEdgePulse_ = 0;
    fineHomeMidPulse_ = 0;
    fineHomeElapsedMs_ = 0;
    fineHomeStage_ = FineHomeStage::SeekFirstEdge;

    boardAdapter_.setPollIntervalMs(5);
    fineHomeTimer_.setInterval(5);
    if (!motionService_.jogStart(+1, fineHomeJogSpeedPulseMs_)) {
        lblHomeResult_->setText(QStringLiteral("回零结果: 精零点启动失败。"));
        lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
        logService_.error(QStringLiteral("回零"), QStringLiteral("精零点启动失败: 无法进入点动。"));
        boardAdapter_.setPollIntervalMs(200);
        return false;
    }

    fineHoming_ = true;
    fineHomeTimer_.start();
    lblHomeResult_->setText(QStringLiteral("回零结果: 精零点进行中..."));
    lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
    updateBoardDependentUiState();

    logService_.info(
        QStringLiteral("回零"),
        QStringLiteral("精零点开始: DI%1, active_low=%2, 正向扫描, 速度=%3 °/s, 初始状态=%4。")
            .arg(kFineHomeDiIndex)
            .arg(appConfig_.fineHomeActiveLow ? QStringLiteral("true") : QStringLiteral("false"))
            .arg(fineHomeJogSpeedPulseMs_ * 1000.0 / appConfig_.pulsePerDeg, 0, 'f', 3)
            .arg(fineHomeLastLaserState_ ? QStringLiteral("TRIGGERED") : QStringLiteral("CLEAR")));
    logService_.info(
        QStringLiteral("回零"),
        QStringLiteral("精零点限制: 速度为当前点动的%1%%, 最大行程=%2°, 最大脉冲=%3, 超时=%4ms。")
            .arg(static_cast<int>(kFineHomeJogSpeedScale * 100))
            .arg(appConfig_.fineHomeMaxTravelDeg, 0, 'f', 3)
            .arg(static_cast<qint64>(qRound64(appConfig_.fineHomeMaxTravelDeg * appConfig_.pulsePerDeg)))
            .arg(appConfig_.fineHomeTimeoutMs));
    return true;
}

void MainWindow::finishCoarseHoming(bool success, const QString& message) {
    coarseHomeTimer_.stop();
    if (coarseHoming_) {
        motionService_.stop(!success);
    }
    boardAdapter_.setPollIntervalMs(200);
    coarseHomeTimer_.setInterval(5);
    coarseHoming_ = false;
    coarseHomeElapsedMs_ = 0;
    coarseHomeConfirmRemainMs_ = 0;
    coarseHomeStartPulse_ = 0;
    coarseHomeStage_ = CoarseHomeStage::Idle;
    coarseHomeConfirmPulse_ = 0;
    btnCoarseCheck_->setEnabled(true);
    btnFineHome_->setEnabled(true);

    bool finalSuccess = success;
    QString finalMessage = message;
    if (success) {
        QString error;
        if (!boardAdapter_.zeroAxis(appConfig_.axisIndex, &error)) {
            finalSuccess = false;
            finalMessage = QStringLiteral("置零失败: %1").arg(error);
        } else {
            motionService_.refreshPosition();
        }
    }

    lblHomeResult_->setText(QStringLiteral("回零结果: %1").arg(finalMessage));
    if (finalSuccess) {
        lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
        logService_.info(QStringLiteral("回零"), finalMessage);
    } else {
        logService_.error(QStringLiteral("回零"), finalMessage);
    }
    updateBoardDependentUiState();
}

void MainWindow::finishFineHoming(bool success, const QString& message) {
    fineHomeTimer_.stop();
    if (fineHoming_ && motionService_.stateText() != QStringLiteral("READY")) {
        motionService_.stop(!success);
    }
    boardAdapter_.setPollIntervalMs(200);
    fineHoming_ = false;
    fineHomeElapsedMs_ = 0;
    fineHomeStartPulse_ = 0;
    fineHomeStage_ = FineHomeStage::Idle;
    fineHomeJogSpeedPulseMs_ = 0.0;
    fineHomeLastLaserState_ = laserTriggered_;

    lblHomeResult_->setText(QStringLiteral("回零结果: %1").arg(message));
    if (success) {
        lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): 0.000"));
        logService_.info(QStringLiteral("回零"), message);
    } else {
        lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
        logService_.error(QStringLiteral("回零"), message);
    }
    updateBoardDependentUiState();
}

void MainWindow::handleSafetyTrip(const QString& reason) {
    if (safetyTripped_) {
        return;
    }
    safetyTripped_ = true;

    if (syncPlaybackService_.isRunning()) {
        stopSyncWorkflow(false);
    }
    if (coarseHoming_) {
        finishCoarseHoming(false, QStringLiteral("安全联锁中断: %1").arg(reason));
    } else if (fineHoming_) {
        finishFineHoming(false, QStringLiteral("安全联锁中断: %1").arg(reason));
    } else if (motionService_.stateText() != QStringLiteral("READY")) {
        motionService_.stop(true);
    }

    lblSyncState_->setText(QStringLiteral("同步状态: ALARM"));
    logService_.error(QStringLiteral("安全"), QStringLiteral("联锁触发: %1").arg(reason));
    updateBoardDependentUiState();
}

void MainWindow::updateSyncElapsedLabel(qint64 elapsedMs) {
    if (lblSyncElapsed_ == nullptr) {
        return;
    }

    qint64 effectiveMs = elapsedMs;
    if (effectiveMs < 0) {
        effectiveMs = syncElapsedTimer_.isValid() ? syncElapsedTimer_.elapsed() : syncElapsedMs_;
    }
    if (effectiveMs < 0) {
        effectiveMs = 0;
    }

    const qint64 totalSec = effectiveMs / 1000;
    const qint64 hour = totalSec / 3600;
    const qint64 minute = (totalSec % 3600) / 60;
    const qint64 second = totalSec % 60;
    lblSyncElapsed_->setText(
        QStringLiteral("同步耗时: %1:%2:%3")
            .arg(hour, 2, 10, QLatin1Char('0'))
            .arg(minute, 2, 10, QLatin1Char('0'))
            .arg(second, 2, 10, QLatin1Char('0')));
}

bool MainWindow::autoConnectBoard() {
    if (boardAdapter_.isConnected()) {
        const QString openedPort = boardAdapter_.currentPort().trimmed().toUpper();
        if (comboBoardPort_ != nullptr && !openedPort.isEmpty()) {
            const int index = comboBoardPort_->findText(openedPort);
            if (index >= 0) {
                comboBoardPort_->setCurrentIndex(index);
            }
        }
        return true;
    }

    auto normalizeCom = [](QString port) {
        port = port.trimmed().toUpper();
        if (port.isEmpty()) {
            return port;
        }
        bool ok = false;
        const int number = port.toInt(&ok);
        if (ok && number > 0) {
            return QStringLiteral("COM%1").arg(number);
        }
        if (!port.startsWith(QStringLiteral("COM"))) {
            return QStringLiteral("COM%1").arg(port);
        }
        return port;
    };

    auto isValidComPort = [](const QString& port) {
        if (!port.startsWith(QStringLiteral("COM"))) {
            return false;
        }
        const QString suffix = port.mid(3);
        if (suffix.isEmpty()) {
            return false;
        }
        for (const QChar ch : suffix) {
            if (!ch.isDigit()) {
                return false;
            }
        }
        return true;
    };

    QStringList candidates;
    auto appendCandidate = [&candidates, &normalizeCom, &isValidComPort](const QString& port) {
        const QString trimmed = port.trimmed();
        if (trimmed.isEmpty() || trimmed == QStringLiteral("无可用串口")) {
            return;
        }
        const QString normalized = normalizeCom(trimmed);
        if (!isValidComPort(normalized) || candidates.contains(normalized)) {
            return;
        }
        candidates.push_back(normalized);
    };

    appendCandidate(lastBoardPort_);
    for (const QString& port : boardAdapter_.autoDetectPorts()) {
        appendCandidate(port);
    }
    appendCandidate(comboBoardPort_ != nullptr ? comboBoardPort_->currentText() : QString());
    if (candidates.isEmpty()) {
        logService_.warn(QStringLiteral("板卡"),
                         QStringLiteral("未找到可尝试的板卡串口候选，已跳过自动连接。"));
        return false;
    }

    for (const QString& port : candidates) {
        logService_.info(QStringLiteral("板卡"), QStringLiteral("尝试自动连接板卡串口: %1").arg(port));
        if (boardAdapter_.connectBoard(port)) {
            if (comboBoardPort_ != nullptr) {
                const int index = comboBoardPort_->findText(port);
                if (index >= 0) {
                    comboBoardPort_->setCurrentIndex(index);
                }
            }
            logService_.info(QStringLiteral("板卡"), QStringLiteral("板卡自动识别成功: %1").arg(port));
            return true;
        }
    }

    return false;
}

bool MainWindow::autoConnectProjector() {
    if (projectorService_.isPortOpen()) {
        const QString openedPort = projectorService_.currentPortName().trimmed().toUpper();
        if (comboProjectorPort_ != nullptr && !openedPort.isEmpty()) {
            const int index = comboProjectorPort_->findText(openedPort);
            if (index >= 0) {
                comboProjectorPort_->setCurrentIndex(index);
            }
        }
        return true;
    }

    auto normalizeCom = [](QString port) {
        port = port.trimmed().toUpper();
        if (port.isEmpty()) {
            return port;
        }
        bool ok = false;
        const int number = port.toInt(&ok);
        if (ok && number > 0) {
            return QStringLiteral("COM%1").arg(number);
        }
        if (!port.startsWith(QStringLiteral("COM"))) {
            return QStringLiteral("COM%1").arg(port);
        }
        return port;
    };

    QStringList candidates;
    auto appendCandidate = [&candidates, &normalizeCom](const QString& port) {
        const QString normalized = normalizeCom(port);
        if (!normalized.isEmpty() && normalized != QStringLiteral("无可用串口") &&
            !candidates.contains(normalized)) {
            candidates.push_back(normalized);
        }
    };

    const QString boardPort =
        boardAdapter_.isConnected() ? normalizeCom(boardAdapter_.currentPort()) : QString();

    auto appendCandidateIfNotBoard = [&](const QString& port) {
        const QString normalized = normalizeCom(port);
        if (!boardPort.isEmpty() && normalized == boardPort) {
            return;
        }
        appendCandidate(normalized);
    };

    appendCandidateIfNotBoard(lastProjectorPort_);
    for (const QString& port : projectorService_.autoDetectPorts()) {
        appendCandidateIfNotBoard(port);
    }
    appendCandidateIfNotBoard(comboProjectorPort_ != nullptr ? comboProjectorPort_->currentText()
                                                             : QString());

    if (candidates.isEmpty()) {
        logService_.warn(QStringLiteral("光机"),
                         QStringLiteral("未找到可尝试的光机串口候选，已跳过自动连接。"));
        return false;
    }

    QString lastError;
    for (const QString& port : candidates) {
        QString error;
        logService_.info(QStringLiteral("光机"), QStringLiteral("尝试自动连接光机串口: %1").arg(port));
        if (projectorService_.openPort(port, &error)) {
            if (comboProjectorPort_ != nullptr) {
                const int index = comboProjectorPort_->findText(port);
                if (index >= 0) {
                    comboProjectorPort_->setCurrentIndex(index);
                }
            }
            logService_.info(QStringLiteral("光机"), QStringLiteral("光机自动识别成功: %1").arg(port));
            return true;
        }
        if (!error.isEmpty()) {
            lastError = error;
        }
    }

    if (!lastError.isEmpty()) {
        logService_.error(QStringLiteral("光机"), lastError);
    }
    return false;
}

void MainWindow::shutdownProjector(bool powerOff) {
    QString error;
    if (!projectorService_.setLed(false, &error)) {
        logService_.error(QStringLiteral("光机"), QStringLiteral("关闭LED失败: %1").arg(error));
    }
    if (powerOff) {
        error.clear();
        if (!projectorService_.setPower(false, &error)) {
            logService_.error(QStringLiteral("光机"), QStringLiteral("关闭光机失败: %1").arg(error));
        }
    }
}

void MainWindow::stopSyncWorkflow(bool powerOffProjector) {
    if (!syncPlaybackService_.isRunning()) {
        projectionWindow_.hideProjection();
        btnSyncStart_->setEnabled(true);
        btnSyncStop_->setEnabled(false);
        updateBoardDependentUiState();
        return;
    }

    syncPlaybackService_.stop(false);
    shutdownProjector(powerOffProjector);
    projectionWindow_.hideProjection();
    btnSyncStart_->setEnabled(true);
    btnSyncStop_->setEnabled(false);
    updateBoardDependentUiState();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    syncElapsedUiTimer_.stop();
    if (syncPlaybackService_.isRunning()) {
        stopSyncWorkflow(false);
    }
    projectionWindow_.clearAlignment();
    projectionWindow_.hideProjection();
    projectionWindow_.close();
    shutdownProjector(false);
    projectorService_.closePort();
    cameraService_.closeCamera();
    boardAdapter_.disconnectBoard();
    QMainWindow::closeEvent(event);
}

void MainWindow::updateBoardDependentUiState() {
    const bool connected = boardAdapter_.isConnected();
    const bool homingBusy = coarseHoming_ || fineHoming_;
    const bool syncRunning = syncPlaybackService_.isRunning();
    const bool canHome = connected && !safetyTripped_ && !homingBusy && !syncRunning;

    if (motionSectionWidget_ != nullptr) {
        motionSectionWidget_->setEnabled(connected && !safetyTripped_);
    }
    if (btnCoarseCheck_ != nullptr) {
        btnCoarseCheck_->setEnabled(canHome);
    }
    if (btnFineHome_ != nullptr) {
        btnFineHome_->setEnabled(canHome);
    }
    if (btnConfirmZReady_ != nullptr) {
        btnConfirmZReady_->setEnabled(
            connected && !safetyTripped_ && !homingBusy && !syncRunning && laserStableForZConfirm());
    }
    if (btnSyncStart_ != nullptr) {
        btnSyncStart_->setEnabled(connected && !safetyTripped_ && !syncRunning && !homingBusy);
    }
    if (btnSyncStop_ != nullptr && !syncRunning) {
        btnSyncStop_->setEnabled(false);
    }
}

void MainWindow::updateJogButtonStyle() {
    if (btnJogPlusMotion_ == nullptr || btnJogMinusMotion_ == nullptr) {
        return;
    }
    const QString normalStyle = QStringLiteral(
        "QPushButton{background:#f7f9fc;border:1px solid #c9d1da;border-radius:6px;"
        "padding:0 14px;min-height:36px;}"
        "QPushButton:hover{background:#eef3fa;}"
        "QPushButton:pressed{background:#e2eaf5;}");
    const QString activeStyle = QStringLiteral(
        "QPushButton{background:#dff6df;border:1px solid #6fbf73;border-radius:6px;"
        "padding:0 14px;min-height:36px;font-weight:600;}"
        "QPushButton:hover{background:#d1f0d1;}"
        "QPushButton:pressed{background:#c4eac4;}");
    btnJogPlusMotion_->setStyleSheet(jogUiDirection_ > 0 ? activeStyle : normalStyle);
    btnJogMinusMotion_->setStyleSheet(jogUiDirection_ < 0 ? activeStyle : normalStyle);
}

bool MainWindow::laserTriggeredFromRaw(quint32 raw) const {
    const bool rawBit = ((raw >> kFineHomeDiIndex) & 0x1U) != 0;
    return appConfig_.fineHomeActiveLow ? !rawBit : rawBit;
}

bool MainWindow::laserStableForZConfirm() const {
    if (!boardAdapter_.isConnected() || !laserTriggered_ || !laserStableTimer_.isValid()) {
        return false;
    }
    return laserStableTimer_.elapsed() >= appConfig_.laserStableMs;
}

void MainWindow::updateLaserState(quint32 raw) {
    const bool logicalTriggered = boardAdapter_.isConnected() ? laserTriggeredFromRaw(raw) : false;
    if (!laserStableTimer_.isValid()) {
        laserTriggered_ = logicalTriggered;
        laserStableTimer_.start();
    } else if (logicalTriggered != laserTriggered_) {
        laserTriggered_ = logicalTriggered;
        laserStableTimer_.restart();
    }

    if (lblLaserState_ == nullptr) {
        updateBoardDependentUiState();
        return;
    }

    if (!boardAdapter_.isConnected()) {
        lblLaserState_->setText(QStringLiteral("对射状态: 未连接"));
        lblLaserState_->setStyleSheet(
            QStringLiteral("QLabel{background:#f5f7fa;border:1px solid #dbe2ea;border-radius:6px;padding:4px 8px;}"));
    } else if (laserTriggered_) {
        const QString stateText = laserStableForZConfirm() ? QStringLiteral("已触发(稳定)")
                                                           : QStringLiteral("已触发(稳定中)");
        lblLaserState_->setText(
            QStringLiteral("对射状态(DI%1): %2").arg(kFineHomeDiIndex).arg(stateText));
        lblLaserState_->setStyleSheet(
            QStringLiteral("QLabel{background:#ecfdf3;border:1px solid #a6f4c5;color:#067647;border-radius:6px;padding:4px 8px;font-weight:600;}"));
    } else {
        lblLaserState_->setText(
            QStringLiteral("对射状态(DI%1): 未触发").arg(kFineHomeDiIndex));
        lblLaserState_->setStyleSheet(
            QStringLiteral("QLabel{background:#fff5f5;border:1px solid #e1b9b9;color:#9f1239;border-radius:6px;padding:4px 8px;font-weight:600;}"));
    }

    updateBoardDependentUiState();
}

void MainWindow::updateDiIndicators(quint32 raw) {
    for (int i = 0; i < diIndicators_.size(); ++i) {
        const bool on = (raw >> i) & 0x1U;
        diIndicators_[i]->setText(QStringLiteral("DI%1  %2").arg(i).arg(on ? "ON" : "OFF"));
        diIndicators_[i]->setStyleSheet(
            on ? QStringLiteral("QLabel{background:#dff6df;border:1px solid #6fbf73;border-radius:6px;padding:4px 10px;font-weight:600;}")
               : QStringLiteral("QLabel{background:#fff5f5;border:1px solid #e1b9b9;border-radius:6px;padding:4px 10px;font-weight:600;}"));
    }
    updateLaserState(raw);
}

void MainWindow::appendUiLog(const QString& message) {
    if (textLog_ != nullptr) {
        textLog_->appendPlainText(message);
    }
}

void MainWindow::chooseCameraSaveDirectory() {
    const QString initial =
        editCameraSaveDir_ != nullptr && !editCameraSaveDir_->text().trimmed().isEmpty()
            ? editCameraSaveDir_->text().trimmed()
            : cameraService_.saveDirectory();
    const QString folder =
        QFileDialog::getExistingDirectory(this, QStringLiteral("选择相机保存目录"), initial);
    if (folder.isEmpty()) {
        return;
    }

    cameraService_.setSaveDirectory(folder);
    if (editCameraSaveDir_ != nullptr) {
        editCameraSaveDir_->setText(QDir::toNativeSeparators(cameraService_.saveDirectory()));
    }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == editCameraSaveDir_ && editCameraSaveDir_ != nullptr) {
        if (event->type() == QEvent::MouseButtonPress) {
            chooseCameraSaveDirectory();
            return true;
        }
        if (event->type() == QEvent::KeyPress) {
            const auto* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter ||
                keyEvent->key() == Qt::Key_Space) {
                chooseCameraSaveDirectory();
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

bool MainWindow::resolveJogSpeed(double& outSpeedPulseMs) {
    if (editJogSpeedPulseMs_ == nullptr) {
        outSpeedPulseMs = qBound(1.0, appConfig_.velMaxPulseMs * 0.8, appConfig_.velMaxPulseMs);
        return true;
    }
    double speedDegPerSec = editJogSpeedPulseMs_->value();
    if (speedDegPerSec <= 0.0) {
        logService_.warn(QStringLiteral("输入"), QStringLiteral("速度必须大于 0。"));
        return false;
    }

    const double maxSpeedDegPerSec =
        appConfig_.velMaxPulseMs * 1000.0 / appConfig_.pulsePerDeg;
    if (speedDegPerSec > maxSpeedDegPerSec) {
        logService_.warn(QStringLiteral("输入"),
                         QStringLiteral("速度超过上限，已限制到 %1 °/s。")
                             .arg(maxSpeedDegPerSec, 0, 'f', 3));
        speedDegPerSec = maxSpeedDegPerSec;
        editJogSpeedPulseMs_->setValue(speedDegPerSec);
    }

    if (speedDegPerSec >= maxSpeedDegPerSec * 0.9) {
        const auto reply = QMessageBox::question(
            this, QStringLiteral("速度确认"),
            QStringLiteral("当前速度 %1 °/s 接近上限，确认继续？").arg(speedDegPerSec, 0, 'f', 3),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return false;
        }
    }

    outSpeedPulseMs = speedDegPerSec * appConfig_.pulsePerDeg / 1000.0;
    return true;
}

QWidget* MainWindow::createBoardMotionPanel() {
    auto* box = new QGroupBox(QStringLiteral("板卡与运动"));
    box->setObjectName(QStringLiteral("boxBoardMotion"));
    auto* root = new QVBoxLayout(box);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto* connection = new QGridLayout();
    connection->setHorizontalSpacing(8);
    connection->setVerticalSpacing(8);

    btnConnectCard_ = new QPushButton(QStringLiteral("连接板卡"));
    btnConnectCard_->setObjectName(QStringLiteral("btnConnectCard"));
    btnConnectCard_->setProperty("role", QStringLiteral("primary"));
    btnConnectCard_->setMinimumWidth(110);
    comboBoardPort_ = new QComboBox();
    comboBoardPort_->setObjectName(QStringLiteral("comboBoardPort"));
    comboBoardPort_->setEditable(false);
    btnBoardRefresh_ = new QPushButton(QStringLiteral("刷新串口"));
    btnBoardRefresh_->setObjectName(QStringLiteral("btnBoardRefresh"));

    lblCardStatus_ = new QLabel(QStringLiteral("板卡状态: 未连接"));
    lblCardStatus_->setObjectName(QStringLiteral("lblCardStatus"));
    lblCardStatus_->setStyleSheet(
        QStringLiteral("QLabel{background:#eef4ff;border:1px solid #c8daf8;border-radius:6px;padding:4px 8px;}"));

    lblComPort_ = new QLabel(QStringLiteral("板卡串口: N/A"));
    lblComPort_->setObjectName(QStringLiteral("lblComPort"));
    lblComPort_->setVisible(false);

    connection->addWidget(new QLabel(QStringLiteral("板卡串口")), 0, 0);
    connection->addWidget(comboBoardPort_, 0, 1, 1, 2);
    connection->addWidget(btnBoardRefresh_, 0, 3);
    connection->addWidget(btnConnectCard_, 0, 4);
    connection->addWidget(lblCardStatus_, 1, 0, 1, 5);
    root->addLayout(connection);

    auto* dioRow = new QHBoxLayout();
    dioRow->setSpacing(8);
    diIndicators_.clear();
    for (int i = 0; i < 2; ++i) {
        auto* indicator = new QLabel(QStringLiteral("DI%1  OFF").arg(i));
        indicator->setObjectName(QStringLiteral("diIndicator[%1]").arg(i));
        indicator->setAlignment(Qt::AlignCenter);
        indicator->setMinimumSize(120, 40);
        indicator->setStyleSheet(
            QStringLiteral("QLabel{background:#fff5f5;border:1px solid #e1b9b9;border-radius:6px;padding:4px 10px;font-weight:600;}"));
        dioRow->addWidget(indicator);
        diIndicators_.push_back(indicator);
    }
    dioRow->addStretch(1);
    root->addLayout(dioRow);

    motionSectionWidget_ = new QWidget(box);
    auto* controlsLayout = new QVBoxLayout(motionSectionWidget_);
    controlsLayout->setContentsMargins(0, 0, 0, 0);
    controlsLayout->setSpacing(10);

    auto* motionGrid = new QGridLayout();
    motionGrid->setHorizontalSpacing(8);
    motionGrid->setVerticalSpacing(8);

    btnJogPlusMotion_ = new QPushButton(QStringLiteral("点动+"));
    btnJogPlusMotion_->setObjectName(QStringLiteral("btnJogPlus"));
    btnJogMinusMotion_ = new QPushButton(QStringLiteral("点动-"));
    btnJogMinusMotion_->setObjectName(QStringLiteral("btnJogMinus"));
    btnStopMotion_ = new QPushButton(QStringLiteral("停止"));
    btnStopMotion_->setObjectName(QStringLiteral("btnStop"));
    btnEStopMotion_ = new QPushButton(QStringLiteral("急停"));
    btnEStopMotion_->setObjectName(QStringLiteral("btnEStop"));
    btnEStopMotion_->setProperty("role", QStringLiteral("danger"));
    btnEStopMotion_->setMinimumWidth(110);

    editJogSpeedPulseMs_ = new QDoubleSpinBox();
    editJogSpeedPulseMs_->setObjectName(QStringLiteral("editJogSpeedPulseMs"));
    editJogSpeedPulseMs_->setRange(0.001, 100000.0);
    editJogSpeedPulseMs_->setDecimals(3);
    editJogSpeedPulseMs_->setSingleStep(1.0);
    editJogSpeedPulseMs_->setSuffix(QStringLiteral(" °/s"));

    editTargetAbsDeg_ = new QDoubleSpinBox();
    editTargetAbsDeg_->setObjectName(QStringLiteral("editTargetAbsDeg"));
    editTargetAbsDeg_->setRange(-360000.0, 360000.0);
    editTargetAbsDeg_->setDecimals(3);
    editTargetAbsDeg_->setSingleStep(1.0);
    editTargetAbsDeg_->setSuffix(QStringLiteral(" °"));
    btnMoveAbsMotion_ = new QPushButton(QStringLiteral("绝对移动"));
    btnMoveAbsMotion_->setObjectName(QStringLiteral("btnMoveAbs"));

    editTargetRelDeg_ = new QDoubleSpinBox();
    editTargetRelDeg_->setObjectName(QStringLiteral("editTargetRelDeg"));
    editTargetRelDeg_->setRange(-360000.0, 360000.0);
    editTargetRelDeg_->setDecimals(3);
    editTargetRelDeg_->setSingleStep(1.0);
    editTargetRelDeg_->setSuffix(QStringLiteral(" °"));
    btnMoveRelMotion_ = new QPushButton(QStringLiteral("相对移动"));
    btnMoveRelMotion_->setObjectName(QStringLiteral("btnMoveRel"));

    lblCurrentDeg_ = new QLabel(QStringLiteral("当前角度: 0.000°"));
    lblCurrentDeg_->setObjectName(QStringLiteral("lblCurrentDeg"));
    lblCurrentPulse_ = new QLabel(QStringLiteral("当前位置脉冲: 0"));
    lblCurrentPulse_->setObjectName(QStringLiteral("lblCurrentPulse"));
    lblMotionConfig_ = new QLabel(QStringLiteral("参数: -"));
    lblMotionConfig_->setObjectName(QStringLiteral("lblMotionConfig"));

    motionGrid->addWidget(btnJogPlusMotion_, 0, 0);
    motionGrid->addWidget(btnJogMinusMotion_, 0, 1);
    motionGrid->addWidget(btnStopMotion_, 0, 2);
    motionGrid->addWidget(btnEStopMotion_, 0, 3);
    motionGrid->addWidget(new QLabel(QStringLiteral("速度(°/s)")), 1, 0);
    motionGrid->addWidget(editJogSpeedPulseMs_, 1, 1, 1, 3);
    motionGrid->addWidget(new QLabel(QStringLiteral("绝对角度")), 2, 0);
    motionGrid->addWidget(editTargetAbsDeg_, 2, 1, 1, 2);
    motionGrid->addWidget(btnMoveAbsMotion_, 2, 3);
    motionGrid->addWidget(new QLabel(QStringLiteral("相对角度")), 3, 0);
    motionGrid->addWidget(editTargetRelDeg_, 3, 1, 1, 2);
    motionGrid->addWidget(btnMoveRelMotion_, 3, 3);
    motionGrid->addWidget(lblCurrentDeg_, 4, 0, 1, 2);
    motionGrid->addWidget(lblCurrentPulse_, 4, 2, 1, 2);
    motionGrid->addWidget(lblMotionConfig_, 5, 0, 1, 4);
    controlsLayout->addLayout(motionGrid);

    auto* homeGrid = new QGridLayout();
    homeGrid->setHorizontalSpacing(8);
    homeGrid->setVerticalSpacing(8);

    btnCoarseCheck_ = new QPushButton(QStringLiteral("粗零点"));
    btnCoarseCheck_->setObjectName(QStringLiteral("btnCoarseCheck"));
    btnFineHome_ = new QPushButton(QStringLiteral("精零点"));
    btnFineHome_->setObjectName(QStringLiteral("btnFineHome"));
    lblLaserState_ = new QLabel(QStringLiteral("对射状态: 未连接"));
    lblLaserState_->setObjectName(QStringLiteral("lblLaserState"));
    lblLaserState_->setStyleSheet(
        QStringLiteral("QLabel{background:#f5f7fa;border:1px solid #dbe2ea;border-radius:6px;padding:4px 8px;}"));

    btnConfirmZReady_ = new QPushButton(QStringLiteral("确认Z到位"));
    btnConfirmZReady_->setObjectName(QStringLiteral("btnConfirmZReady"));
    btnConfirmZReady_->setProperty("role", QStringLiteral("primary"));
    btnConfirmZReady_->setEnabled(false);

    lblHomeResult_ = new QLabel(QStringLiteral("回零结果: N/A"));
    lblHomeResult_->setObjectName(QStringLiteral("lblHomeResult"));
    lblZeroErrorDeg_ = new QLabel(QStringLiteral("零点误差(°): N/A"));
    lblZeroErrorDeg_->setObjectName(QStringLiteral("lblZeroErrorDeg"));
    lblZConfirmTime_ = new QLabel(QStringLiteral("Z确认时间: N/A"));
    lblZConfirmTime_->setObjectName(QStringLiteral("lblZConfirmTime"));

    homeGrid->addWidget(btnCoarseCheck_, 0, 0);
    homeGrid->addWidget(btnFineHome_, 0, 1);
    homeGrid->addWidget(lblLaserState_, 0, 2, 1, 2);
    homeGrid->addWidget(btnConfirmZReady_, 1, 0, 1, 2);
    controlsLayout->addLayout(homeGrid);

    root->addWidget(motionSectionWidget_, 1);
    return box;
}

QWidget* MainWindow::createProjectorSyncPanel() {
    auto* box = new QGroupBox(QStringLiteral("光机与同步投影"));
    box->setObjectName(QStringLiteral("boxProjectorSync"));
    auto* root = new QVBoxLayout(box);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto* connection = new QGridLayout();
    connection->setHorizontalSpacing(8);
    connection->setVerticalSpacing(8);

    comboProjectorPort_ = new QComboBox();
    comboProjectorPort_->setObjectName(QStringLiteral("comboProjectorPort"));
    btnProjectorRefresh_ = new QPushButton(QStringLiteral("刷新串口"));
    btnProjectorRefresh_->setObjectName(QStringLiteral("btnRefreshProjectorPort"));
    btnProjectorOpen_ = new QPushButton(QStringLiteral("连接光机"));
    btnProjectorOpen_->setObjectName(QStringLiteral("btnProjectorOpen"));
    btnProjectorClose_ = new QPushButton(QStringLiteral("断开光机"));
    btnProjectorClose_->setObjectName(QStringLiteral("btnProjectorClose"));
    lblProjectorPortState_ = new QLabel(QStringLiteral("光机串口状态: 未连接"));
    lblProjectorPortState_->setObjectName(QStringLiteral("lblProjectorPortState"));
    lblProjectorPortState_->setStyleSheet(
        QStringLiteral("QLabel{background:#eef4ff;border:1px solid #c8daf8;border-radius:6px;padding:4px 8px;}"));

    connection->addWidget(new QLabel(QStringLiteral("光机串口")), 0, 0);
    connection->addWidget(comboProjectorPort_, 0, 1, 1, 2);
    connection->addWidget(btnProjectorRefresh_, 0, 3);
    connection->addWidget(btnProjectorOpen_, 0, 4);
    connection->addWidget(btnProjectorClose_, 0, 5);
    connection->addWidget(lblProjectorPortState_, 1, 0, 1, 6);
    root->addLayout(connection);

    auto* basicGrid = new QGridLayout();
    basicGrid->setHorizontalSpacing(8);
    basicGrid->setVerticalSpacing(8);

    btnProjectorPowerOn_ = new QPushButton(QStringLiteral("光机开"));
    btnProjectorPowerOn_->setObjectName(QStringLiteral("btnProjectorPowerOn"));
    btnProjectorPowerOff_ = new QPushButton(QStringLiteral("光机关"));
    btnProjectorPowerOff_->setObjectName(QStringLiteral("btnProjectorPowerOff"));
    btnLedOn_ = new QPushButton(QStringLiteral("LED开"));
    btnLedOn_->setObjectName(QStringLiteral("btnLedOn"));
    btnLedOff_ = new QPushButton(QStringLiteral("LED关"));
    btnLedOff_->setObjectName(QStringLiteral("btnLedOff"));
    btnAlignSiemens_ = new QPushButton(QStringLiteral("光机对准"));
    btnAlignSiemens_->setObjectName(QStringLiteral("btnAlignSiemens"));

    editLedPercent_ = new QSpinBox();
    editLedPercent_->setObjectName(QStringLiteral("editLedPercent"));
    editLedPercent_->setRange(0, 100);
    editLedPercent_->setSuffix(QStringLiteral(" %"));
    btnApplyLedPercent_ = new QPushButton(QStringLiteral("应用亮度"));
    btnApplyLedPercent_->setObjectName(QStringLiteral("btnApplyLedPercent"));

    basicGrid->addWidget(btnProjectorPowerOn_, 0, 0);
    basicGrid->addWidget(btnProjectorPowerOff_, 0, 1);
    basicGrid->addWidget(btnLedOn_, 0, 2);
    basicGrid->addWidget(btnLedOff_, 0, 3);
    basicGrid->addWidget(new QLabel(QStringLiteral("亮度")), 1, 0);
    basicGrid->addWidget(editLedPercent_, 1, 1);
    basicGrid->addWidget(btnApplyLedPercent_, 1, 2);
    basicGrid->addWidget(btnAlignSiemens_, 1, 3);
    root->addLayout(basicGrid);

    auto* folderGrid = new QGridLayout();
    folderGrid->setHorizontalSpacing(8);
    folderGrid->setVerticalSpacing(8);

    editImageFolder_ = new QLineEdit();
    editImageFolder_->setObjectName(QStringLiteral("editImageFolder"));
    editImageFolder_->setPlaceholderText(
        QStringLiteral("选择图片文件夹（支持 0deg.png 或 0000.png）"));
    btnBrowseImageFolder_ = new QPushButton(QStringLiteral("浏览"));
    btnBrowseImageFolder_->setObjectName(QStringLiteral("btnBrowseImageFolder"));
    btnCheckImageFolder_ = new QPushButton(QStringLiteral("检查序列"));
    btnCheckImageFolder_->setObjectName(QStringLiteral("btnCheckImageFolder"));
    lblImageCheckResult_ = new QLabel(QStringLiteral("序列检查: 未执行"));
    lblImageCheckResult_->setObjectName(QStringLiteral("lblImageCheckResult"));

    folderGrid->addWidget(new QLabel(QStringLiteral("图片目录")), 0, 0);
    folderGrid->addWidget(editImageFolder_, 0, 1, 1, 3);
    folderGrid->addWidget(btnBrowseImageFolder_, 0, 4);
    folderGrid->addWidget(btnCheckImageFolder_, 0, 5);
    folderGrid->addWidget(lblImageCheckResult_, 1, 0, 1, 6);
    root->addLayout(folderGrid);

    auto* syncParamGrid = new QGridLayout();
    syncParamGrid->setHorizontalSpacing(8);
    syncParamGrid->setVerticalSpacing(8);

    spinLoopCount_ = new QSpinBox();
    spinLoopCount_->setObjectName(QStringLiteral("spinLoopCount"));
    spinLoopCount_->setRange(1, 999999);
    spinLoopCount_->setValue(1);
    chkInfiniteLoop_ = new QCheckBox(QStringLiteral("无限循环"));
    chkInfiniteLoop_->setObjectName(QStringLiteral("chkInfiniteLoop"));

    editRoundTimeSec_ = new QDoubleSpinBox();
    editRoundTimeSec_->setObjectName(QStringLiteral("editRoundTimeSec"));
    editRoundTimeSec_->setRange(0.001, 360000.0);
    editRoundTimeSec_->setDecimals(3);
    editRoundTimeSec_->setSingleStep(1.0);
    editRoundTimeSec_->setSuffix(QStringLiteral(" s"));

    syncParamGrid->addWidget(new QLabel(QStringLiteral("循环次数")), 0, 0);
    syncParamGrid->addWidget(spinLoopCount_, 0, 1);
    syncParamGrid->addWidget(chkInfiniteLoop_, 0, 2);
    syncParamGrid->addWidget(new QLabel(QStringLiteral("一圈时间")), 0, 3);
    syncParamGrid->addWidget(editRoundTimeSec_, 0, 4, 1, 2);
    root->addLayout(syncParamGrid);

    auto* syncStatusGrid = new QGridLayout();
    syncStatusGrid->setHorizontalSpacing(8);
    syncStatusGrid->setVerticalSpacing(8);

    btnSyncStart_ = new QPushButton(QStringLiteral("开始同步"));
    btnSyncStart_->setObjectName(QStringLiteral("btnSyncStart"));
    btnSyncStart_->setProperty("role", QStringLiteral("primary"));
    btnSyncStart_->setMinimumWidth(120);
    btnSyncStop_ = new QPushButton(QStringLiteral("停止同步"));
    btnSyncStop_->setObjectName(QStringLiteral("btnSyncStop"));
    btnSyncStop_->setMinimumWidth(120);
    lblCurrentFrame_ = new QLabel(QStringLiteral("当前投影: N/A"));
    lblCurrentFrame_->setObjectName(QStringLiteral("lblCurrentFrame"));
    lblSyncState_ = new QLabel(QStringLiteral("同步状态: READY"));
    lblSyncState_->setObjectName(QStringLiteral("lblSyncState"));
    lblSyncProgress_ = new QLabel(QStringLiteral("同步进度: 0/0"));
    lblSyncProgress_->setObjectName(QStringLiteral("lblSyncProgress"));
    lblSyncElapsed_ = new QLabel(QStringLiteral("同步耗时: 00:00:00"));
    lblSyncElapsed_->setObjectName(QStringLiteral("lblSyncElapsed"));

    syncStatusGrid->addWidget(lblCurrentFrame_, 0, 0, 1, 3);
    syncStatusGrid->addWidget(lblSyncState_, 0, 3, 1, 3);
    syncStatusGrid->addWidget(lblSyncProgress_, 1, 0, 1, 3);
    syncStatusGrid->addWidget(lblSyncElapsed_, 1, 3, 1, 3);
    syncStatusGrid->addWidget(btnSyncStart_, 2, 0);
    syncStatusGrid->addWidget(btnSyncStop_, 2, 1);
    root->addLayout(syncStatusGrid);

    return box;
}

QWidget* MainWindow::createCameraPanel() {
    auto* box = new QGroupBox(QStringLiteral("相机与图像"));
    box->setObjectName(QStringLiteral("boxCamera"));
    auto* root = new QVBoxLayout(box);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    comboCameraList_ = new QComboBox();
    comboCameraList_->setObjectName(QStringLiteral("comboCameraList"));
    comboCameraList_->addItem(QStringLiteral("未检测到相机"));
    btnCameraRefresh_ = new QPushButton(QStringLiteral("刷新相机"));
    btnCameraRefresh_->setObjectName(QStringLiteral("btnCameraRefresh"));
    btnCameraOpen_ = new QPushButton(QStringLiteral("打开相机"));
    btnCameraOpen_->setObjectName(QStringLiteral("btnCameraOpen"));
    btnCameraOpen_->setEnabled(false);

    lblCameraState_ = new QLabel(QStringLiteral("相机状态: 未打开"));
    lblCameraState_->setObjectName(QStringLiteral("lblCameraState"));
    lblCameraState_->setStyleSheet(
        QStringLiteral("QLabel{background:#eef4ff;border:1px solid #c8daf8;border-radius:6px;padding:4px 8px;}"));
    lblCameraRecordTime_ = new QLabel(QStringLiteral("录像时长: 00:00:00"));
    lblCameraRecordTime_->setObjectName(QStringLiteral("lblCameraRecordTime"));
    lblCameraRecordTime_->setStyleSheet(
        QStringLiteral("QLabel{background:#f5f7fa;border:1px solid #dbe2ea;border-radius:6px;padding:4px 8px;}"));

    auto* topBar = new QHBoxLayout();
    topBar->setSpacing(8);
    topBar->addWidget(comboCameraList_, 2);
    topBar->addWidget(btnCameraRefresh_);
    topBar->addWidget(btnCameraOpen_);
    topBar->addWidget(lblCameraState_);
    topBar->addWidget(lblCameraRecordTime_);
    root->addLayout(topBar);

    auto* previewBox = new QGroupBox(QStringLiteral("相机预览"));
    previewBox->setObjectName(QStringLiteral("boxCameraPreview"));
    auto* previewLayout = new QVBoxLayout(previewBox);
    previewLayout->setContentsMargins(12, 12, 12, 12);
    previewLayout->setSpacing(10);

    QWidget* preview = cameraService_.previewWidget();
    preview->setObjectName(QStringLiteral("cameraPreviewWidget"));
    preview->setMinimumHeight(320);
    preview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    previewLayout->addWidget(preview, 1);
    root->addWidget(previewBox, 1);

    btnSnap_ = new QPushButton(QStringLiteral("拍照"));
    btnSnap_->setObjectName(QStringLiteral("btnSnap"));
    btnSnap_->setEnabled(false);
    btnRec_ = new QPushButton(QStringLiteral("录像"));
    btnRec_->setObjectName(QStringLiteral("btnRec"));
    btnRec_->setEnabled(false);
    btnRec_->setStyleSheet(
        QStringLiteral("QPushButton{background:#f0f0f0;color:#222;border:1px solid #999;}"
                       "QPushButton:pressed{background:#e2e2e2;}"));
    btnCameraModeToggle_ = new QPushButton(QStringLiteral("模式: 彩色"));
    btnCameraModeToggle_->setObjectName(QStringLiteral("btnCameraModeToggle"));
    btnCameraModeToggle_->setEnabled(false);

    auto* previewBtnRow = new QHBoxLayout();
    previewBtnRow->setSpacing(8);
    previewBtnRow->addWidget(btnSnap_);
    previewBtnRow->addWidget(btnRec_);
    previewBtnRow->addWidget(btnCameraModeToggle_);
    previewBtnRow->addStretch(1);
    root->addLayout(previewBtnRow);

    auto* paramBox = new QGroupBox(QStringLiteral("参数区"));
    paramBox->setObjectName(QStringLiteral("boxCameraParams"));
    auto* paramLayout = new QFormLayout(paramBox);
    paramBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    paramLayout->setContentsMargins(12, 12, 12, 12);
    paramLayout->setHorizontalSpacing(10);
    paramLayout->setVerticalSpacing(8);
    paramLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    editCameraSaveDir_ = new QLineEdit();
    editCameraSaveDir_->setObjectName(QStringLiteral("editCameraSaveDir"));
    editCameraSaveDir_->setReadOnly(true);
    editCameraSaveDir_->setPlaceholderText(QStringLiteral("拍照/录像保存目录"));
    editCameraSaveDir_->setCursor(Qt::PointingHandCursor);
    editCameraSaveDir_->setToolTip(QStringLiteral("点击选择相机保存目录"));
    editCameraSaveDir_->installEventFilter(this);

    spinCameraBrightness_ = new QDoubleSpinBox();
    spinCameraBrightness_->setObjectName(QStringLiteral("spinCameraBrightness"));
    spinCameraBrightness_->setRange(-1.0, 1.0);
    spinCameraBrightness_->setDecimals(3);
    spinCameraBrightness_->setSingleStep(0.1);
    spinCameraBrightness_->setEnabled(false);

    spinCameraContrast_ = new QDoubleSpinBox();
    spinCameraContrast_->setObjectName(QStringLiteral("spinCameraContrast"));
    spinCameraContrast_->setRange(-1.0, 1.0);
    spinCameraContrast_->setDecimals(3);
    spinCameraContrast_->setSingleStep(0.1);
    spinCameraContrast_->setEnabled(false);

    spinCameraSaturation_ = new QDoubleSpinBox();
    spinCameraSaturation_->setObjectName(QStringLiteral("spinCameraSaturation"));
    spinCameraSaturation_->setRange(-1.0, 1.0);
    spinCameraSaturation_->setDecimals(3);
    spinCameraSaturation_->setSingleStep(0.1);
    spinCameraSaturation_->setEnabled(false);

    spinCameraExposure_ = new QDoubleSpinBox();
    spinCameraExposure_->setObjectName(QStringLiteral("spinCameraExposure"));
    spinCameraExposure_->setRange(-1.0, 1.0);
    spinCameraExposure_->setDecimals(3);
    spinCameraExposure_->setSingleStep(0.1);
    spinCameraExposure_->setEnabled(false);

    paramLayout->addRow(QStringLiteral("保存目录"), editCameraSaveDir_);
    paramLayout->addRow(QStringLiteral("亮度"), spinCameraBrightness_);
    paramLayout->addRow(QStringLiteral("对比度"), spinCameraContrast_);
    paramLayout->addRow(QStringLiteral("饱和度"), spinCameraSaturation_);
    paramLayout->addRow(QStringLiteral("曝光补偿"), spinCameraExposure_);
    root->addWidget(paramBox);

    return box;
}

QWidget* MainWindow::createProjectionPreviewPanel() {
    auto* box = new QGroupBox(QStringLiteral("实时投影预览"));
    box->setObjectName(QStringLiteral("boxProjectionPreview"));
    auto* layout = new QVBoxLayout(box);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    lblProjectionPreview_ = new QLabel(QStringLiteral("实时投影预览"));
    lblProjectionPreview_->setObjectName(QStringLiteral("lblProjectionPreview"));
    lblProjectionPreview_->setAlignment(Qt::AlignCenter);
    lblProjectionPreview_->setMinimumHeight(340);
    lblProjectionPreview_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lblProjectionPreview_->setStyleSheet(
        QStringLiteral("QLabel{background:#111;border:1px solid #555;color:#ddd;border-radius:6px;}"));
    lblProjectionPreview_->setScaledContents(false);
    layout->addWidget(lblProjectionPreview_, 1);
    return box;
}

QWidget* MainWindow::createLogPanel() {
    auto* box = new QGroupBox(QStringLiteral("日志"));
    auto* layout = new QVBoxLayout(box);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    textLog_ = new QPlainTextEdit();
    textLog_->setObjectName(QStringLiteral("textLog"));
    textLog_->setReadOnly(true);
    textLog_->setMinimumHeight(120);
    textLog_->setMaximumHeight(140);
    textLog_->appendPlainText(QStringLiteral("[信息] UI 骨架已初始化。"));
    layout->addWidget(textLog_);
    return box;
}
