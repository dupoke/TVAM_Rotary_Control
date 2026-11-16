#include "SyncPlaybackService.h"

#include "MotionService.h"

#include <QtGlobal>
#include <cmath>
#include <limits>
#include <utility>

namespace {

double normalizeRelativeDeg(double value) {
    double mod = std::fmod(value, 360.0);
    if (mod < 0.0) {
        mod += 360.0;
    }
    return mod;
}

}

SyncPlaybackService::SyncPlaybackService(MotionService* motionService, QObject* parent)
    : QObject(parent),
      motionService_(motionService) {
    Q_ASSERT(motionService_ != nullptr);
    connect(motionService_, &MotionService::positionChanged,
            this, &SyncPlaybackService::onMotionPositionChanged);
    connect(motionService_, &MotionService::motionFinished,
            this, &SyncPlaybackService::onMotionFinished);
}

void SyncPlaybackService::setFrames(QVector<ImageFrame> frames) {
    frames_ = std::move(frames);
}

bool SyncPlaybackService::start(double roundTimeSec, int loopCount, bool infinite, QString* error) {
    if (running_) {
        if (error != nullptr) {
            *error = QStringLiteral("同步流程已在运行。");
        }
        return false;
    }
    if (frames_.isEmpty()) {
        if (error != nullptr) {
            *error = QStringLiteral("未加载可用图片序列。");
        }
        return false;
    }
    if (roundTimeSec <= 0.0) {
        if (error != nullptr) {
            *error = QStringLiteral("一圈时间必须大于0。");
        }
        return false;
    }
    if (!infinite && loopCount <= 0) {
        if (error != nullptr) {
            *error = QStringLiteral("循环次数必须大于0。");
        }
        return false;
    }

    motionService_->refreshPosition();
    running_ = true;
    infinite_ = infinite;
    loopTarget_ = infinite ? 0 : loopCount;
    currentLoop_ = 0;
    nextFrameIndex_ = 0;
    startPulse_ = motionService_->currentPulse();
    roundTimeSec_ = roundTimeSec;

    if (!motionService_->startSynchronizedRotation(roundTimeSec_, loopCount, infinite_)) {
        running_ = false;
        if (error != nullptr) {
            *error = QStringLiteral("启动单圈定长旋转失败，请检查一圈时间和板卡速度上限。");
        }
        return false;
    }

    emit syncLog(QStringLiteral("同步启动：每圈定长 360°，目标 %1 圈，一圈 %2 s。")
                     .arg(infinite_ ? QStringLiteral("∞") : QString::number(loopTarget_))
                     .arg(roundTimeSec_, 0, 'f', 3));
    emit stateChanged(QStringLiteral("SYNC_RUNNING"));
    emit progressChanged(currentLoop_, loopTarget_, 0.0);
    return true;
}

void SyncPlaybackService::stop(bool completed) {
    if (!running_) {
        return;
    }

    running_ = false;

    // 无论是否完成，都立即停止运动
    // 不进行额外的回零动作，让停止点更精准
    motionService_->stop(false);

    emit stateChanged(QStringLiteral("READY"));
    emit syncFinished(completed);
}

bool SyncPlaybackService::isRunning() const {
    return running_;
}

int SyncPlaybackService::currentLoop() const {
    return currentLoop_;
}

int SyncPlaybackService::loopTarget() const {
    return loopTarget_;
}

void SyncPlaybackService::onMotionPositionChanged(qint64 pulse, double, double cycleDeg) {
    if (!running_ || frames_.isEmpty()) {
        return;
    }

    const qint64 turnPulses = qRound64(360.0 * motionService_->pulsePerDeg());
    if (turnPulses <= 0) {
        return;
    }

    qint64 relativePulse = pulse - startPulse_;
    if (relativePulse < 0) {
        return;
    }
    const qint64 requestedPulse = infinite_ ? relativePulse : turnPulses * loopTarget_;
    relativePulse = qMin(relativePulse, requestedPulse);
    const int completedLoops = static_cast<int>(qMin(
        static_cast<double>(infinite_ ? std::numeric_limits<int>::max() : loopTarget_),
        static_cast<double>(relativePulse / turnPulses)));
    const int playbackLoop = infinite_
                                 ? completedLoops
                                 : qMin(qMax(0, completedLoops), qMax(0, loopTarget_ - 1));
    if (playbackLoop != currentLoop_) {
        currentLoop_ = playbackLoop;
        nextFrameIndex_ = 0;
    }

    while (nextFrameIndex_ < frames_.size()) {
        const qint64 framePulse = qRound64(frames_[nextFrameIndex_].angleDeg *
                                           motionService_->pulsePerDeg());
        const qint64 triggerPulse = static_cast<qint64>(currentLoop_) * turnPulses + framePulse;
        if (relativePulse < triggerPulse) {
            break;
        }

        const ImageFrame& frame = frames_[nextFrameIndex_];
        emit frameChanged(nextFrameIndex_, currentLoop_, frame.filePath, frame.angleDeg);
        ++nextFrameIndex_;
    }

    Q_UNUSED(cycleDeg);
    const double relativeDeg = static_cast<double>(relativePulse) / motionService_->pulsePerDeg();
    emit progressChanged(completedLoops, loopTarget_, normalizeRelativeDeg(relativeDeg));
}

void SyncPlaybackService::onMotionFinished() {
    if (!running_) {
        return;
    }
    if (!motionService_->positionTargetReached()) {
        emit syncLog(QStringLiteral("第 %1 圈未确认到达目标位置，已停止同步。")
                         .arg(currentLoop_ + 1));
        stop(false);
        return;
    }

    currentLoop_ = loopTarget_;
    emit progressChanged(currentLoop_, loopTarget_, 0.0);
    emit syncLog(QStringLiteral("已完成 %1 圈，运动到达总目标位置。")
                     .arg(loopTarget_));
    stop(true);
}
