#include "MotionService.h"

#include "BoardAdapter.h"

#include <QtMath>
#include <cmath>

namespace {
constexpr long kAxisRunningBit = 0x00000400;
}

MotionService::MotionService(QObject* parent)
    : QObject(parent) {
    tickTimer_.setInterval(20);
    connect(&tickTimer_, &QTimer::timeout, this, &MotionService::onTick);
}

void MotionService::setPulsePerDeg(double value) {
    if (value <= 0.0) {
        emit motionError(QStringLiteral("脉冲当量必须大于0。"));
        return;
    }
    pulsePerDeg_ = value;
}

void MotionService::setMaxVelocityPulsePerMs(double value) {
    if (value <= 0.0) {
        emit motionError(QStringLiteral("最大速度参数无效。"));
        return;
    }
    maxVelPulsePerSec_ = value * 1000.0;
    jogVelPulsePerSec_ = qMin(maxVelPulsePerSec_, 2000.0);
}

void MotionService::setBoardAdapter(BoardAdapter* adapter) {
    boardAdapter_ = adapter;
}

void MotionService::setAxisIndex(int axisIndex) {
    if (axisIndex < 1 || axisIndex > 16) {
        emit motionError(QStringLiteral("轴号配置无效: %1。").arg(axisIndex));
        return;
    }
    axisIndex_ = axisIndex;
}

bool MotionService::jogStart(int direction) {
    return jogStart(direction, -1.0);
}

bool MotionService::jogStart(int direction, double velPulsePerMs) {
    if (direction == 0) {
        emit motionError(QStringLiteral("点动方向无效。"));
        return false;
    }
    jogDirection_ = direction > 0 ? 1 : -1;
    const double requestedPulsePerSec = velPulsePerMs > 0.0
                                            ? velPulsePerMs * 1000.0
                                            : jogVelPulsePerSec_;
    activeJogVelPulsePerSec_ = qBound(1.0, requestedPulsePerSec, maxVelPulsePerSec_);

    if (useHardware()) {
        QString error;
        if (!boardAdapter_->jogStart(axisIndex_, jogDirection_, activeJogVelPulsePerSec_ / 1000.0,
                                     accPulsePerMs2_, decPulsePerMs2_, &error)) {
            emit motionError(error);
            return false;
        }
        setMode(Mode::Jogging);
        syncPositionFromHardware();
        emit motionLog(QStringLiteral("点动开始(硬件)，轴%1，方向: %2，速度: %3 脉冲/ms")
                           .arg(axisIndex_)
                           .arg(jogDirection_ > 0 ? "+" : "-")
                           .arg(activeJogVelPulsePerSec_ / 1000.0, 0, 'f', 3));
        return true;
    }

    setMode(Mode::Jogging);
    emit motionLog(QStringLiteral("点动开始，方向: %1，速度: %2 脉冲/ms")
                       .arg(jogDirection_ > 0 ? "+" : "-")
                       .arg(activeJogVelPulsePerSec_ / 1000.0, 0, 'f', 3));
    return true;
}

bool MotionService::moveToAbsDeg(double targetDeg) {
    const qint64 target = static_cast<qint64>(qRound64(targetDeg * pulsePerDeg_));
    targetPulse_ = target;

    if (useHardware()) {
        QString error;
        if (!boardAdapter_->moveToPulse(axisIndex_, targetPulse_, maxVelPulsePerSec_ / 1000.0,
                                        accPulsePerMs2_, decPulsePerMs2_, &error)) {
            emit motionError(error);
            return false;
        }
        setMode(Mode::Positioning);
        syncPositionFromHardware();
        emit motionLog(QStringLiteral("绝对移动(硬件)到 %1°").arg(targetDeg, 0, 'f', 3));
        return true;
    }

    setMode(Mode::Positioning);
    emit motionLog(QStringLiteral("绝对移动到 %1°").arg(targetDeg, 0, 'f', 3));
    return true;
}

bool MotionService::moveByRelDeg(double deltaDeg) {
    const qint64 deltaPulse = static_cast<qint64>(qRound64(deltaDeg * pulsePerDeg_));
    targetPulse_ = currentPulse_ + deltaPulse;

    if (useHardware()) {
        QString error;
        if (!boardAdapter_->moveToPulse(axisIndex_, targetPulse_, maxVelPulsePerSec_ / 1000.0,
                                        accPulsePerMs2_, decPulsePerMs2_, &error)) {
            emit motionError(error);
            return false;
        }
        setMode(Mode::Positioning);
        syncPositionFromHardware();
        emit motionLog(QStringLiteral("相对移动(硬件) %1°").arg(deltaDeg, 0, 'f', 3));
        return true;
    }

    setMode(Mode::Positioning);
    emit motionLog(QStringLiteral("相对移动 %1°").arg(deltaDeg, 0, 'f', 3));
    return true;
}

bool MotionService::startContinuous(double roundTimeSec) {
    if (roundTimeSec <= 0.0) {
        emit motionError(QStringLiteral("一圈时间必须大于0。"));
        return false;
    }
    const double degPerSec = 360.0 / roundTimeSec;
    continuousPulsePerSec_ = degPerSec * pulsePerDeg_;
    if (continuousPulsePerSec_ <= 0.0) {
        emit motionError(QStringLiteral("连续旋转速度计算失败。"));
        return false;
    }

    if (useHardware()) {
        QString error;
        if (!boardAdapter_->jogStart(axisIndex_, +1, continuousPulsePerSec_ / 1000.0,
                                     accPulsePerMs2_, decPulsePerMs2_, &error)) {
            emit motionError(error);
            return false;
        }
        setMode(Mode::Continuous);
        syncPositionFromHardware();
        emit motionLog(QStringLiteral("连续旋转开始(硬件)，一圈时间: %1 s").arg(roundTimeSec, 0, 'f', 3));
        return true;
    }

    setMode(Mode::Continuous);
    emit motionLog(QStringLiteral("连续旋转开始，一圈时间: %1 s").arg(roundTimeSec, 0, 'f', 3));
    return true;
}

void MotionService::stop(bool estop) {
    const bool wasRunning = mode_ != Mode::Idle;
    if (wasRunning && useHardware()) {
        QString error;
        if (!boardAdapter_->stopAxis(axisIndex_, estop, &error)) {
            emit motionError(error);
        }
    }
    setMode(Mode::Idle);
    if (wasRunning) {
        if (useHardware()) {
            syncPositionFromHardware();
        }
        emit motionLog(estop ? QStringLiteral("急停触发。") : QStringLiteral("运动停止。"));
        emit motionFinished();
    }
}

qint64 MotionService::currentPulse() const {
    return currentPulse_;
}

double MotionService::currentTotalDeg() const {
    return currentPulse_ / pulsePerDeg_;
}

double MotionService::currentCycleDeg() const {
    return normalizeDeg(currentTotalDeg());
}

QString MotionService::stateText() const {
    switch (mode_) {
    case Mode::Idle:
        return QStringLiteral("READY");
    case Mode::Jogging:
        return QStringLiteral("JOGGING");
    case Mode::Positioning:
        return QStringLiteral("POSITIONING");
    case Mode::Continuous:
        return QStringLiteral("SYNC_RUNNING");
    default:
        return QStringLiteral("READY");
    }
}

double MotionService::normalizeDeg(double value) {
    double mod = std::fmod(value, 360.0);
    if (mod < 0.0) {
        mod += 360.0;
    }
    return mod;
}

void MotionService::setMode(Mode mode) {
    if (mode_ == mode) {
        return;
    }
    mode_ = mode;
    if (mode_ == Mode::Idle) {
        tickTimer_.stop();
    } else if (!tickTimer_.isActive()) {
        tickTimer_.start();
    }
    emit stateChanged(stateText());
}

bool MotionService::useHardware() const {
    return boardAdapter_ != nullptr && boardAdapter_->isConnected();
}

void MotionService::syncPositionFromHardware() {
    if (!useHardware()) {
        return;
    }

    double pulse = 0.0;
    QString error;
    if (!boardAdapter_->getProfilePositionPulse(axisIndex_, pulse, &error)) {
        ++hardwareReadErrorCount_;
        if (hardwareReadErrorCount_ == 1 || (hardwareReadErrorCount_ % 20) == 0) {
            emit motionError(error);
        }
        return;
    }

    hardwareReadErrorCount_ = 0;
    currentPulse_ = static_cast<qint64>(qRound64(pulse));
    emit positionChanged(currentPulse_, currentTotalDeg(), currentCycleDeg());
}

void MotionService::onTick() {
    const double dtSec = tickTimer_.interval() / 1000.0;
    if (dtSec <= 0.0) {
        return;
    }

    if (useHardware()) {
        if (mode_ == Mode::Idle) {
            return;
        }
        syncPositionFromHardware();

        if (mode_ == Mode::Positioning) {
            long sts = 0;
            QString error;
            if (!boardAdapter_->getAxisStatus(axisIndex_, sts, &error)) {
                emit motionError(error);
                return;
            }
            const bool running = (sts & kAxisRunningBit) != 0;
            if (!running) {
                setMode(Mode::Idle);
                emit motionFinished();
                emit motionLog(QStringLiteral("定位完成。"));
            }
        }
        return;
    }

    switch (mode_) {
    case Mode::Idle:
        return;
    case Mode::Jogging: {
        const double deltaPulse = jogDirection_ * activeJogVelPulsePerSec_ * dtSec;
        currentPulse_ += static_cast<qint64>(qRound64(deltaPulse));
        break;
    }
    case Mode::Positioning: {
        const qint64 diff = targetPulse_ - currentPulse_;
        if (diff == 0) {
            setMode(Mode::Idle);
            emit motionFinished();
            break;
        }
        const qint64 stepLimit = static_cast<qint64>(qRound64(maxVelPulsePerSec_ * dtSec));
        const qint64 step = qBound<qint64>(-stepLimit, diff, stepLimit);
        currentPulse_ += step;
        if (currentPulse_ == targetPulse_) {
            setMode(Mode::Idle);
            emit motionFinished();
            emit motionLog(QStringLiteral("定位完成。"));
        }
        break;
    }
    case Mode::Continuous: {
        const double deltaPulse = continuousPulsePerSec_ * dtSec;
        currentPulse_ += static_cast<qint64>(qRound64(deltaPulse));
        break;
    }
    }

    emit positionChanged(currentPulse_, currentTotalDeg(), currentCycleDeg());
}
