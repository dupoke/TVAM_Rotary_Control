#include "MotionService.h"

#include <QtMath>
#include <cmath>

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

bool MotionService::jogStart(int direction) {
    if (direction == 0) {
        emit motionError(QStringLiteral("点动方向无效。"));
        return false;
    }
    jogDirection_ = direction > 0 ? 1 : -1;
    setMode(Mode::Jogging);
    emit motionLog(QStringLiteral("点动开始，方向: %1").arg(jogDirection_ > 0 ? "+" : "-"));
    return true;
}

bool MotionService::moveToAbsDeg(double targetDeg) {
    const qint64 target = static_cast<qint64>(qRound64(targetDeg * pulsePerDeg_));
    targetPulse_ = target;
    setMode(Mode::Positioning);
    emit motionLog(QStringLiteral("绝对移动到 %1°").arg(targetDeg, 0, 'f', 3));
    return true;
}

bool MotionService::moveByRelDeg(double deltaDeg) {
    const qint64 deltaPulse = static_cast<qint64>(qRound64(deltaDeg * pulsePerDeg_));
    targetPulse_ = currentPulse_ + deltaPulse;
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
    setMode(Mode::Continuous);
    emit motionLog(QStringLiteral("连续旋转开始，一圈时间: %1 s").arg(roundTimeSec, 0, 'f', 3));
    return true;
}

void MotionService::stop(bool estop) {
    const bool wasRunning = mode_ != Mode::Idle;
    setMode(Mode::Idle);
    if (wasRunning) {
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

void MotionService::onTick() {
    const double dtSec = tickTimer_.interval() / 1000.0;
    if (dtSec <= 0.0) {
        return;
    }

    switch (mode_) {
    case Mode::Idle:
        return;
    case Mode::Jogging: {
        const double deltaPulse = jogDirection_ * jogVelPulsePerSec_ * dtSec;
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
