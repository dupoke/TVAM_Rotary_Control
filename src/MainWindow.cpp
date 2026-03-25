#include "MainWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      configManager_(QStringLiteral("config/app_config.ini")),
      logService_(QStringLiteral("logs")),
      syncPlaybackService_(&motionService_) {
    setupUi();
    initServices();
    wireSignals();
}

void MainWindow::setupUi() {
    setWindowTitle(QStringLiteral("旋转台控制软件"));
    resize(1440, 920);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(10);

    auto* topGrid = new QGridLayout();
    topGrid->setHorizontalSpacing(10);
    topGrid->setVerticalSpacing(10);

    topGrid->addWidget(createDeviceSection(), 0, 0);
    topGrid->addWidget(createMotionSection(), 0, 1);
    topGrid->addWidget(createHomingSection(), 1, 0);
    topGrid->addWidget(createZConfirmSection(), 1, 1);

    root->addLayout(topGrid);
    root->addWidget(createCameraSection());
    root->addWidget(createProjectorSyncSection());
    root->addWidget(createLogSection(), 1);
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

    motionService_.setPulsePerDeg(appConfig_.pulsePerDeg);
    motionService_.setMaxVelocityPulsePerMs(appConfig_.velMaxPulseMs);
    motionService_.setBoardAdapter(&boardAdapter_);
    motionService_.setAxisIndex(appConfig_.axisIndex);
    projectorService_.applyConfig(appConfig_);

    editRoundTimeSec_->setText(QStringLiteral("60"));
    editLedPercent_->setText(QStringLiteral("30"));
    lblSyncState_->setText(QStringLiteral("同步状态: READY"));
    lblSyncProgress_->setText(QStringLiteral("同步进度: 0/0"));
    lblCurrentFrame_->setText(QStringLiteral("当前投影: N/A"));

    btnSyncStop_->setEnabled(false);
    boardPortScanTimer_.setInterval(1500);
    coarseHomeTimer_.setInterval(50);
    refreshBoardPorts();
    refreshProjectorPorts();
    updateDiIndicators(0);
}

void MainWindow::wireSignals() {
    connect(&boardAdapter_, &BoardAdapter::boardLog, this, [this](const QString& msg) {
        logService_.info(QStringLiteral("板卡"), msg);
    });
    connect(&boardAdapter_, &BoardAdapter::connectionChanged, this,
            [this](bool connected, const QString& portName) {
                lblCardStatus_->setText(
                    connected ? QStringLiteral("板卡状态: 已连接") : QStringLiteral("板卡状态: 未连接"));
                lblComPort_->setText(QStringLiteral("板卡串口: %1").arg(portName));
                btnConnectCard_->setText(connected ? QStringLiteral("断开板卡")
                                                  : QStringLiteral("连接板卡"));
                if (!connected) {
                    if (coarseHoming_) {
                        finishCoarseHoming(false, QStringLiteral("粗零点中断: 板卡已断开。"));
                    } else if (motionService_.stateText() != QStringLiteral("READY")) {
                        motionService_.stop(true);
                    }
                }
            });
    connect(&boardAdapter_, &BoardAdapter::diChanged, this,
            [this](quint32 raw) { updateDiIndicators(raw); });

    connect(&motionService_, &MotionService::motionLog, this,
            [this](const QString& msg) { logService_.info(QStringLiteral("运动"), msg); });
    connect(&motionService_, &MotionService::motionError, this,
            [this](const QString& msg) { logService_.error(QStringLiteral("运动"), msg); });
    connect(&motionService_, &MotionService::positionChanged, this,
            [this](qint64 pulse, double, double cycleDeg) {
                lblCurrentPulse_->setText(QStringLiteral("当前位置脉冲: %1").arg(pulse));
                lblCurrentDeg_->setText(QStringLiteral("当前角度: %1°").arg(cycleDeg, 0, 'f', 3));
            });

    connect(&projectorService_, &ProjectorService::projectorLog, this,
            [this](const QString& msg) { logService_.info(QStringLiteral("光机"), msg); });
    connect(&projectorService_, &ProjectorService::portStateChanged, this,
            [this](bool opened, const QString& portName) {
                lblProjectorPortState_->setText(
                    opened ? QStringLiteral("光机串口状态: 已连接(%1)").arg(portName)
                           : QStringLiteral("光机串口状态: 未连接"));
            });

    connect(&syncPlaybackService_, &SyncPlaybackService::syncLog, this,
            [this](const QString& msg) { logService_.info(QStringLiteral("同步"), msg); });
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
                logService_.info(QStringLiteral("同步"),
                                 QStringLiteral("切换投影: %1").arg(filePath));
            });
    connect(&syncPlaybackService_, &SyncPlaybackService::syncFinished, this, [this]() {
        btnSyncStart_->setEnabled(true);
        btnSyncStop_->setEnabled(false);
        projectionWindow_.hideProjection();
    });

    connect(btnConnectCard_, &QPushButton::clicked, this, [this]() {
        if (boardAdapter_.isConnected()) {
            boardAdapter_.disconnectBoard();
        } else {
            const QString selectedPort = comboBoardPort_ != nullptr
                                             ? comboBoardPort_->currentText().trimmed()
                                             : QString();
            if (selectedPort.isEmpty()) {
                logService_.warn(QStringLiteral("板卡"), QStringLiteral("请先选择板卡串口。"));
                return;
            }
            if (!boardAdapter_.connectBoard(selectedPort)) {
                logService_.error(QStringLiteral("板卡"), QStringLiteral("板卡连接失败。"));
            }
        }
    });
    connect(btnBoardRefresh_, &QPushButton::clicked, this, [this]() { refreshBoardPorts(); });
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
        if (boardAdapter_.diBit(1)) {
            finishCoarseHoming(true, QStringLiteral("粗零点完成: DI1 已触发。"));
            return;
        }
        if (coarseHomeElapsedMs_ >= appConfig_.homeTimeoutMs) {
            finishCoarseHoming(false, QStringLiteral("粗零点失败: 超时未触发 DI1。"));
        }
    });
    boardPortScanTimer_.start();

    auto* btnJogPlus = findChild<QPushButton*>(QStringLiteral("btnJogPlus"));
    auto* btnJogMinus = findChild<QPushButton*>(QStringLiteral("btnJogMinus"));
    auto* btnStop = findChild<QPushButton*>(QStringLiteral("btnStop"));
    auto* btnEStop = findChild<QPushButton*>(QStringLiteral("btnEStop"));
    auto* btnMoveAbs = findChild<QPushButton*>(QStringLiteral("btnMoveAbs"));
    auto* btnMoveRel = findChild<QPushButton*>(QStringLiteral("btnMoveRel"));

    connect(btnJogPlus, &QPushButton::pressed, this, [this]() { motionService_.jogStart(+1); });
    connect(btnJogMinus, &QPushButton::pressed, this, [this]() { motionService_.jogStart(-1); });
    connect(btnStop, &QPushButton::clicked, this, [this]() {
        motionService_.stop(false);
        if (coarseHoming_) {
            finishCoarseHoming(false, QStringLiteral("粗零点已手动停止。"));
        }
    });
    connect(btnEStop, &QPushButton::clicked, this, [this]() {
        motionService_.stop(true);
        if (coarseHoming_) {
            finishCoarseHoming(false, QStringLiteral("粗零点已急停中断。"));
        }
    });

    connect(btnMoveAbs, &QPushButton::clicked, this, [this]() {
        double value = 0.0;
        if (!parseDoubleInput(editTargetAbsDeg_, QStringLiteral("绝对角度"), value)) {
            return;
        }
        motionService_.moveToAbsDeg(value);
    });

    connect(btnMoveRel, &QPushButton::clicked, this, [this]() {
        double value = 0.0;
        if (!parseDoubleInput(editTargetRelDeg_, QStringLiteral("相对角度"), value)) {
            return;
        }
        motionService_.moveByRelDeg(value);
    });

    connect(btnCoarseCheck_, &QPushButton::clicked, this, [this]() {
        startCoarseHoming();
    });
    connect(btnFineHome_, &QPushButton::clicked, this, [this]() {
        lblHomeResult_->setText(QStringLiteral("回零结果: 精零点待硬件安装后启用。"));
        lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
        logService_.info(QStringLiteral("回零"),
                         QStringLiteral("精零点功能暂未启用，等待对射开关硬件到位。"));
    });

    auto* btnConfirmZReady = findChild<QPushButton*>(QStringLiteral("btnConfirmZReady"));
    connect(btnConfirmZReady, &QPushButton::clicked, this, [this]() {
        const QString now = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        lblZConfirmTime_->setText(QStringLiteral("Z确认时间: %1").arg(now));
        logService_.info(QStringLiteral("Z确认"), QStringLiteral("操作员确认Z到位。"));
    });

    connect(btnProjectorRefresh_, &QPushButton::clicked, this, [this]() {
        refreshProjectorPorts();
    });
    connect(btnProjectorOpen_, &QPushButton::clicked, this, [this]() {
        QString error;
        if (!projectorService_.openPort(comboProjectorPort_->currentText(), &error)) {
            logService_.error(QStringLiteral("光机"), error);
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
        int percent = 0;
        if (!parseIntInput(editLedPercent_, QStringLiteral("亮度百分比"), percent)) {
            return;
        }
        if (percent < 0 || percent > 100) {
            logService_.warn(QStringLiteral("光机"), QStringLiteral("亮度百分比超出范围，已限制在0~100。"));
            percent = qBound(0, percent, 100);
            editLedPercent_->setText(QString::number(percent));
        }
        QString error;
        if (!projectorService_.setBrightnessPercent(percent, &error)) {
            logService_.error(QStringLiteral("光机"), error);
        }
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
        if (!runImageFolderCheck()) {
            logService_.warn(QStringLiteral("同步"), QStringLiteral("同步启动失败: 图片序列无效。"));
            return;
        }

        double roundSec = 0.0;
        if (!parseDoubleInput(editRoundTimeSec_, QStringLiteral("一圈时间"), roundSec) || roundSec <= 0.0) {
            logService_.warn(QStringLiteral("同步"), QStringLiteral("同步启动失败: 一圈时间无效。"));
            return;
        }

        syncPlaybackService_.setFrames(checkedFrames_);
        QString error;
        if (!syncPlaybackService_.start(roundSec, spinLoopCount_->value(),
                                        chkInfiniteLoop_->isChecked(), &error)) {
            logService_.error(QStringLiteral("同步"), error);
            return;
        }

        projectionWindow_.showForProjection();
        btnSyncStart_->setEnabled(false);
        btnSyncStop_->setEnabled(true);
    });

    connect(btnSyncStop_, &QPushButton::clicked, this, [this]() {
        syncPlaybackService_.stop(false);
        btnSyncStart_->setEnabled(true);
        btnSyncStop_->setEnabled(false);
        projectionWindow_.hideProjection();
    });
    connect(chkInfiniteLoop_, &QCheckBox::toggled, this,
            [this](bool checked) { spinLoopCount_->setEnabled(!checked); });
}

void MainWindow::refreshProjectorPorts() {
    const QString current = comboProjectorPort_->currentText();
    comboProjectorPort_->clear();

    const QStringList ports = projectorService_.availablePorts();
    if (ports.isEmpty()) {
        comboProjectorPort_->addItem(QStringLiteral("无可用串口"));
        btnProjectorOpen_->setEnabled(false);
    } else {
        comboProjectorPort_->addItems(ports);
        btnProjectorOpen_->setEnabled(true);
        const int index = ports.indexOf(current);
        if (index >= 0) {
            comboProjectorPort_->setCurrentIndex(index);
        }
    }
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

    QStringList mergedPorts;
    for (int i = 1; i <= 5; ++i) {
        mergedPorts.push_back(QStringLiteral("COM%1").arg(i));
    }

    const QStringList scannedPorts = boardAdapter_.availablePorts();
    for (const QString& port : scannedPorts) {
        const QString normalized = normalizeCom(port);
        if (!normalized.isEmpty() && !mergedPorts.contains(normalized)) {
            mergedPorts.push_back(normalized);
        }
    }

    const QString normalizedCurrent = normalizeCom(current);
    if (!normalizedCurrent.isEmpty() && !mergedPorts.contains(normalizedCurrent)) {
        mergedPorts.push_back(normalizedCurrent);
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
    comboBoardPort_->setCurrentIndex(-1);
}

bool MainWindow::runImageFolderCheck() {
    const QString folderPath = editImageFolder_->text().trimmed();
    if (folderPath.isEmpty()) {
        checkedFrames_.clear();
        lblImageCheckResult_->setText(QStringLiteral("序列检查: 未选择目录。"));
        return false;
    }

    const SequenceCheckResult result = imageSequenceService_.checkFolder(folderPath);
    lblImageCheckResult_->setText(QStringLiteral("序列检查: %1").arg(result.message));
    if (result.ok) {
        checkedFrames_ = result.frames;
        logService_.info(QStringLiteral("序列"), result.message);
    } else {
        checkedFrames_.clear();
        logService_.warn(QStringLiteral("序列"), result.message);
    }
    return result.ok;
}

bool MainWindow::startCoarseHoming() {
    if (coarseHoming_) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("粗零点流程已在运行。"));
        return false;
    }
    if (!boardAdapter_.isConnected()) {
        logService_.warn(QStringLiteral("回零"), QStringLiteral("请先连接板卡后再执行粗零点。"));
        return false;
    }
    if (boardAdapter_.diBit(1)) {
        finishCoarseHoming(true, QStringLiteral("粗零点完成: DI1 已是触发状态。"));
        return true;
    }

    if (!motionService_.jogStart(-1)) {
        lblHomeResult_->setText(QStringLiteral("回零结果: 粗零点启动失败。"));
        logService_.error(QStringLiteral("回零"), QStringLiteral("粗零点启动失败: 无法进入点动。"));
        return false;
    }

    coarseHoming_ = true;
    coarseHomeElapsedMs_ = 0;
    coarseHomeTimer_.start();
    btnCoarseCheck_->setEnabled(false);
    btnFineHome_->setEnabled(false);
    lblHomeResult_->setText(QStringLiteral("回零结果: 粗零点进行中..."));
    lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
    logService_.info(QStringLiteral("回零"),
                     QStringLiteral("粗零点开始: 低速反向旋转等待 DI1 触发。"));
    return true;
}

void MainWindow::finishCoarseHoming(bool success, const QString& message) {
    coarseHomeTimer_.stop();
    if (coarseHoming_) {
        motionService_.stop(!success);
    }
    coarseHoming_ = false;
    coarseHomeElapsedMs_ = 0;
    btnCoarseCheck_->setEnabled(true);
    btnFineHome_->setEnabled(true);

    lblHomeResult_->setText(QStringLiteral("回零结果: %1").arg(message));
    if (success) {
        lblZeroErrorDeg_->setText(QStringLiteral("零点误差(°): N/A"));
        logService_.info(QStringLiteral("回零"), message);
    } else {
        logService_.error(QStringLiteral("回零"), message);
    }
}

void MainWindow::updateDiIndicators(quint32 raw) {
    for (int i = 0; i < diIndicators_.size(); ++i) {
        const bool on = (raw >> i) & 0x1U;
        diIndicators_[i]->setText(QStringLiteral("DI%1: %2").arg(i).arg(on ? "ON" : "OFF"));
        diIndicators_[i]->setStyleSheet(on ? QStringLiteral("QLabel{background:#c8f7c5;border:1px solid #6fbf73;}")
                                           : QStringLiteral("QLabel{background:#f5d0d0;border:1px solid #d08383;}"));
    }
}

void MainWindow::appendUiLog(const QString& message) {
    if (textLog_ != nullptr) {
        textLog_->appendPlainText(message);
    }
}

bool MainWindow::parseDoubleInput(QLineEdit* edit, const QString& fieldName, double& outValue) {
    bool ok = false;
    const double value = edit->text().trimmed().toDouble(&ok);
    if (!ok) {
        logService_.warn(QStringLiteral("输入"), QStringLiteral("%1 输入无效。").arg(fieldName));
        return false;
    }
    outValue = value;
    return true;
}

bool MainWindow::parseIntInput(QLineEdit* edit, const QString& fieldName, int& outValue) {
    bool ok = false;
    const int value = edit->text().trimmed().toInt(&ok);
    if (!ok) {
        logService_.warn(QStringLiteral("输入"), QStringLiteral("%1 输入无效。").arg(fieldName));
        return false;
    }
    outValue = value;
    return true;
}

QWidget* MainWindow::createDeviceSection() {
    auto* box = new QGroupBox(QStringLiteral("设备"));
    auto* layout = new QGridLayout(box);

    btnConnectCard_ = new QPushButton(QStringLiteral("连接板卡"));
    btnConnectCard_->setObjectName(QStringLiteral("btnConnectCard"));
    comboBoardPort_ = new QComboBox();
    comboBoardPort_->setObjectName(QStringLiteral("comboBoardPort"));
    comboBoardPort_->setEditable(false);
    btnBoardRefresh_ = new QPushButton(QStringLiteral("刷新串口"));
    btnBoardRefresh_->setObjectName(QStringLiteral("btnBoardRefresh"));

    lblCardStatus_ = new QLabel(QStringLiteral("板卡状态: 未连接"));
    lblCardStatus_->setObjectName(QStringLiteral("lblCardStatus"));

    lblComPort_ = new QLabel(QStringLiteral("板卡串口: N/A"));
    lblComPort_->setObjectName(QStringLiteral("lblComPort"));

    layout->addWidget(new QLabel(QStringLiteral("板卡串口")), 0, 0);
    layout->addWidget(comboBoardPort_, 0, 1);
    layout->addWidget(btnBoardRefresh_, 0, 2);
    layout->addWidget(btnConnectCard_, 1, 0);
    layout->addWidget(lblCardStatus_, 1, 1, 1, 2);
    layout->addWidget(lblComPort_, 2, 0, 1, 3);

    auto* diRow = new QHBoxLayout();
    diIndicators_.clear();
    for (int i = 0; i < 4; ++i) {
        auto* indicator = new QLabel(QStringLiteral("DI%1: OFF").arg(i));
        indicator->setObjectName(QStringLiteral("diIndicator[%1]").arg(i));
        indicator->setAlignment(Qt::AlignCenter);
        indicator->setFrameShape(QFrame::Box);
        indicator->setMinimumWidth(90);
        diRow->addWidget(indicator);
        diIndicators_.push_back(indicator);
    }
    diRow->addStretch(1);
    layout->addLayout(diRow, 3, 0, 1, 3);
    return box;
}

QWidget* MainWindow::createMotionSection() {
    auto* box = new QGroupBox(QStringLiteral("运动"));
    auto* layout = new QGridLayout(box);

    auto* btnJogPlus = new QPushButton(QStringLiteral("点动+"));
    btnJogPlus->setObjectName(QStringLiteral("btnJogPlus"));
    auto* btnJogMinus = new QPushButton(QStringLiteral("点动-"));
    btnJogMinus->setObjectName(QStringLiteral("btnJogMinus"));
    auto* btnStop = new QPushButton(QStringLiteral("停止"));
    btnStop->setObjectName(QStringLiteral("btnStop"));
    auto* btnEStop = new QPushButton(QStringLiteral("急停"));
    btnEStop->setObjectName(QStringLiteral("btnEStop"));

    editTargetAbsDeg_ = new QLineEdit();
    editTargetAbsDeg_->setObjectName(QStringLiteral("editTargetAbsDeg"));
    editTargetAbsDeg_->setPlaceholderText(QStringLiteral("绝对角度(°)"));
    editTargetAbsDeg_->setValidator(new QDoubleValidator(this));
    auto* btnMoveAbs = new QPushButton(QStringLiteral("绝对移动"));
    btnMoveAbs->setObjectName(QStringLiteral("btnMoveAbs"));

    editTargetRelDeg_ = new QLineEdit();
    editTargetRelDeg_->setObjectName(QStringLiteral("editTargetRelDeg"));
    editTargetRelDeg_->setPlaceholderText(QStringLiteral("相对角度(°)"));
    editTargetRelDeg_->setValidator(new QDoubleValidator(this));
    auto* btnMoveRel = new QPushButton(QStringLiteral("相对移动"));
    btnMoveRel->setObjectName(QStringLiteral("btnMoveRel"));

    lblCurrentDeg_ = new QLabel(QStringLiteral("当前角度: 0.000°"));
    lblCurrentDeg_->setObjectName(QStringLiteral("lblCurrentDeg"));
    lblCurrentPulse_ = new QLabel(QStringLiteral("当前位置脉冲: 0"));
    lblCurrentPulse_->setObjectName(QStringLiteral("lblCurrentPulse"));

    layout->addWidget(btnJogPlus, 0, 0);
    layout->addWidget(btnJogMinus, 0, 1);
    layout->addWidget(btnStop, 0, 2);
    layout->addWidget(btnEStop, 0, 3);

    layout->addWidget(editTargetAbsDeg_, 1, 0, 1, 3);
    layout->addWidget(btnMoveAbs, 1, 3);

    layout->addWidget(editTargetRelDeg_, 2, 0, 1, 3);
    layout->addWidget(btnMoveRel, 2, 3);

    layout->addWidget(lblCurrentDeg_, 3, 0, 1, 2);
    layout->addWidget(lblCurrentPulse_, 3, 2, 1, 2);

    return box;
}

QWidget* MainWindow::createHomingSection() {
    auto* box = new QGroupBox(QStringLiteral("回零"));
    auto* layout = new QGridLayout(box);

    btnCoarseCheck_ = new QPushButton(QStringLiteral("粗零点"));
    btnCoarseCheck_->setObjectName(QStringLiteral("btnCoarseCheck"));
    btnFineHome_ = new QPushButton(QStringLiteral("精零点"));
    btnFineHome_->setObjectName(QStringLiteral("btnFineHome"));

    lblHomeResult_ = new QLabel(QStringLiteral("回零结果: N/A"));
    lblHomeResult_->setObjectName(QStringLiteral("lblHomeResult"));
    lblZeroErrorDeg_ = new QLabel(QStringLiteral("零点误差(°): N/A"));
    lblZeroErrorDeg_->setObjectName(QStringLiteral("lblZeroErrorDeg"));

    layout->addWidget(btnCoarseCheck_, 0, 0);
    layout->addWidget(btnFineHome_, 0, 1);
    layout->addWidget(lblHomeResult_, 1, 0, 1, 2);
    layout->addWidget(lblZeroErrorDeg_, 2, 0, 1, 2);

    return box;
}

QWidget* MainWindow::createZConfirmSection() {
    auto* box = new QGroupBox(QStringLiteral("Z确认"));
    auto* layout = new QGridLayout(box);

    lblLaserState_ = new QLabel(QStringLiteral("激光状态: 未知"));
    lblLaserState_->setObjectName(QStringLiteral("lblLaserState"));

    auto* btnConfirmZReady = new QPushButton(QStringLiteral("确认Z到位"));
    btnConfirmZReady->setObjectName(QStringLiteral("btnConfirmZReady"));

    lblZConfirmTime_ = new QLabel(QStringLiteral("Z确认时间: N/A"));
    lblZConfirmTime_->setObjectName(QStringLiteral("lblZConfirmTime"));

    layout->addWidget(lblLaserState_, 0, 0, 1, 2);
    layout->addWidget(btnConfirmZReady, 1, 0, 1, 2);
    layout->addWidget(lblZConfirmTime_, 2, 0, 1, 2);
    return box;
}

QWidget* MainWindow::createCameraSection() {
    auto* box = new QGroupBox(QStringLiteral("相机"));
    auto* layout = new QGridLayout(box);

    comboCameraList_ = new QComboBox();
    comboCameraList_->setObjectName(QStringLiteral("comboCameraList"));
    comboCameraList_->addItem(QStringLiteral("相机功能待接入"));
    auto* btnCameraOpen = new QPushButton(QStringLiteral("打开相机"));
    btnCameraOpen->setObjectName(QStringLiteral("btnCameraOpen"));

    auto* btnSnap = new QPushButton(QStringLiteral("拍照"));
    btnSnap->setObjectName(QStringLiteral("btnSnap"));
    auto* btnRecStart = new QPushButton(QStringLiteral("开始录像"));
    btnRecStart->setObjectName(QStringLiteral("btnRecStart"));
    auto* btnRecStop = new QPushButton(QStringLiteral("停止录像"));
    btnRecStop->setObjectName(QStringLiteral("btnRecStop"));

    auto* sliderBrightness = new QSlider(Qt::Horizontal);
    sliderBrightness->setObjectName(QStringLiteral("sliderBrightness"));
    sliderBrightness->setRange(0, 100);
    auto* sliderContrast = new QSlider(Qt::Horizontal);
    sliderContrast->setObjectName(QStringLiteral("sliderContrast"));
    sliderContrast->setRange(0, 100);
    auto* sliderExposure = new QSlider(Qt::Horizontal);
    sliderExposure->setObjectName(QStringLiteral("sliderExposure"));
    sliderExposure->setRange(0, 100);
    auto* sliderSaturation = new QSlider(Qt::Horizontal);
    sliderSaturation->setObjectName(QStringLiteral("sliderSaturation"));
    sliderSaturation->setRange(0, 100);

    auto* btnColorMode = new QPushButton(QStringLiteral("彩色"));
    btnColorMode->setObjectName(QStringLiteral("btnColorMode"));
    auto* btnMonoMode = new QPushButton(QStringLiteral("黑白"));
    btnMonoMode->setObjectName(QStringLiteral("btnMonoMode"));

    layout->addWidget(comboCameraList_, 0, 0, 1, 3);
    layout->addWidget(btnCameraOpen, 0, 3);
    layout->addWidget(btnSnap, 1, 0);
    layout->addWidget(btnRecStart, 1, 1);
    layout->addWidget(btnRecStop, 1, 2);
    layout->addWidget(btnColorMode, 1, 3);
    layout->addWidget(btnMonoMode, 1, 4);
    layout->addWidget(new QLabel(QStringLiteral("亮度")), 2, 0);
    layout->addWidget(sliderBrightness, 2, 1, 1, 4);
    layout->addWidget(new QLabel(QStringLiteral("对比度")), 3, 0);
    layout->addWidget(sliderContrast, 3, 1, 1, 4);
    layout->addWidget(new QLabel(QStringLiteral("曝光")), 4, 0);
    layout->addWidget(sliderExposure, 4, 1, 1, 4);
    layout->addWidget(new QLabel(QStringLiteral("饱和度")), 5, 0);
    layout->addWidget(sliderSaturation, 5, 1, 1, 4);
    return box;
}

QWidget* MainWindow::createProjectorSyncSection() {
    auto* box = new QGroupBox(QStringLiteral("光机与同步投影"));
    auto* layout = new QGridLayout(box);

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

    btnProjectorPowerOn_ = new QPushButton(QStringLiteral("光机开"));
    btnProjectorPowerOn_->setObjectName(QStringLiteral("btnProjectorPowerOn"));
    btnProjectorPowerOff_ = new QPushButton(QStringLiteral("光机关"));
    btnProjectorPowerOff_->setObjectName(QStringLiteral("btnProjectorPowerOff"));
    btnLedOn_ = new QPushButton(QStringLiteral("LED开"));
    btnLedOn_->setObjectName(QStringLiteral("btnLedOn"));
    btnLedOff_ = new QPushButton(QStringLiteral("LED关"));
    btnLedOff_->setObjectName(QStringLiteral("btnLedOff"));

    editLedPercent_ = new QLineEdit();
    editLedPercent_->setObjectName(QStringLiteral("editLedPercent"));
    editLedPercent_->setPlaceholderText(QStringLiteral("亮度百分比(0~100)"));
    editLedPercent_->setValidator(new QIntValidator(0, 100, this));
    btnApplyLedPercent_ = new QPushButton(QStringLiteral("应用亮度"));
    btnApplyLedPercent_->setObjectName(QStringLiteral("btnApplyLedPercent"));

    editImageFolder_ = new QLineEdit();
    editImageFolder_->setObjectName(QStringLiteral("editImageFolder"));
    editImageFolder_->setPlaceholderText(QStringLiteral("选择图片文件夹"));
    btnBrowseImageFolder_ = new QPushButton(QStringLiteral("浏览"));
    btnBrowseImageFolder_->setObjectName(QStringLiteral("btnBrowseImageFolder"));
    btnCheckImageFolder_ = new QPushButton(QStringLiteral("检查序列"));
    btnCheckImageFolder_->setObjectName(QStringLiteral("btnCheckImageFolder"));
    lblImageCheckResult_ = new QLabel(QStringLiteral("序列检查: 未执行"));
    lblImageCheckResult_->setObjectName(QStringLiteral("lblImageCheckResult"));
    lblCurrentFrame_ = new QLabel(QStringLiteral("当前投影: N/A"));
    lblCurrentFrame_->setObjectName(QStringLiteral("lblCurrentFrame"));

    spinLoopCount_ = new QSpinBox();
    spinLoopCount_->setObjectName(QStringLiteral("spinLoopCount"));
    spinLoopCount_->setRange(1, 999999);
    spinLoopCount_->setValue(1);
    chkInfiniteLoop_ = new QCheckBox(QStringLiteral("无限循环"));
    chkInfiniteLoop_->setObjectName(QStringLiteral("chkInfiniteLoop"));

    editRoundTimeSec_ = new QLineEdit();
    editRoundTimeSec_->setObjectName(QStringLiteral("editRoundTimeSec"));
    editRoundTimeSec_->setPlaceholderText(QStringLiteral("一圈时间(秒)，例如 60"));
    editRoundTimeSec_->setValidator(new QDoubleValidator(0.001, 360000.0, 6, this));

    btnSyncStart_ = new QPushButton(QStringLiteral("开始同步"));
    btnSyncStart_->setObjectName(QStringLiteral("btnSyncStart"));
    btnSyncStop_ = new QPushButton(QStringLiteral("停止同步"));
    btnSyncStop_->setObjectName(QStringLiteral("btnSyncStop"));
    lblSyncState_ = new QLabel(QStringLiteral("同步状态: READY"));
    lblSyncState_->setObjectName(QStringLiteral("lblSyncState"));
    lblSyncProgress_ = new QLabel(QStringLiteral("同步进度: 0/0"));
    lblSyncProgress_->setObjectName(QStringLiteral("lblSyncProgress"));

    layout->addWidget(new QLabel(QStringLiteral("光机串口")), 0, 0);
    layout->addWidget(comboProjectorPort_, 0, 1, 1, 2);
    layout->addWidget(btnProjectorRefresh_, 0, 3);
    layout->addWidget(btnProjectorOpen_, 0, 4);
    layout->addWidget(btnProjectorClose_, 0, 5);

    layout->addWidget(lblProjectorPortState_, 1, 0, 1, 6);

    layout->addWidget(btnProjectorPowerOn_, 2, 0);
    layout->addWidget(btnProjectorPowerOff_, 2, 1);
    layout->addWidget(btnLedOn_, 2, 2);
    layout->addWidget(btnLedOff_, 2, 3);
    layout->addWidget(editLedPercent_, 2, 4);
    layout->addWidget(btnApplyLedPercent_, 2, 5);

    layout->addWidget(editImageFolder_, 3, 0, 1, 4);
    layout->addWidget(btnBrowseImageFolder_, 3, 4);
    layout->addWidget(btnCheckImageFolder_, 3, 5);

    layout->addWidget(lblImageCheckResult_, 4, 0, 1, 3);
    layout->addWidget(lblCurrentFrame_, 4, 3, 1, 3);

    layout->addWidget(new QLabel(QStringLiteral("循环次数")), 5, 0);
    layout->addWidget(spinLoopCount_, 5, 1);
    layout->addWidget(chkInfiniteLoop_, 5, 2);
    layout->addWidget(new QLabel(QStringLiteral("一圈时间(秒)")), 5, 3);
    layout->addWidget(editRoundTimeSec_, 5, 4, 1, 2);

    layout->addWidget(btnSyncStart_, 6, 0);
    layout->addWidget(btnSyncStop_, 6, 1);
    layout->addWidget(lblSyncState_, 6, 2, 1, 2);
    layout->addWidget(lblSyncProgress_, 6, 4, 1, 2);

    return box;
}

QWidget* MainWindow::createLogSection() {
    auto* box = new QGroupBox(QStringLiteral("日志"));
    auto* layout = new QVBoxLayout(box);

    textLog_ = new QPlainTextEdit();
    textLog_->setObjectName(QStringLiteral("textLog"));
    textLog_->setReadOnly(true);
    textLog_->appendPlainText(QStringLiteral("[信息] UI 骨架已初始化。"));
    layout->addWidget(textLog_);
    return box;
}
