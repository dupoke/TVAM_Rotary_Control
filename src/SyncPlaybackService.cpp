#include "SyncPlaybackService.h"

#include "MotionService.h"

#include <QtGlobal>
#include <cmath>
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
    startTotalDeg_ = motionService_->currentTotalDeg();
    roundTimeSec_ = roundTimeSec;

    if (!motionService_->startRevolution(roundTimeSec_)) {
        running_ = false;
        if (error != nullptr) {
            *error = QStringLiteral("启动单圈定长旋转失败，请检查一圈时间和板卡速度上限。");
        }
        return false;
    }

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

void SyncPlaybackService::onMotionPositionChanged(qint64, double totalDeg, double cycleDeg) {
    if (!running_ || frames_.isEmpty()) {
        return;
    }

    const double relativeDeg = totalDeg - startTotalDeg_;
    if (relativeDeg < 0.0) {
        return;
    }

    while (nextFrameIndex_ < frames_.size()) {
        const double triggerDeg = currentLoop_ * 360.0 + frames_[nextFrameIndex_].angleDeg;
        if (relativeDeg + 1e-6 < triggerDeg) {
            break;
        }

        const ImageFrame& frame = frames_[nextFrameIndex_];
        emit frameChanged(nextFrameIndex_, currentLoop_, frame.filePath, frame.angleDeg);
        ++nextFrameIndex_;
    }

    Q_UNUSED(cycleDeg);
    emit progressChanged(currentLoop_, loopTarget_, normalizeRelativeDeg(relativeDeg));
}

void SyncPlaybackService::onMotionFinished() {
    if (!running_) {
        return;
    }
    if (!motionService_->positionTargetReached()) {
        stop(false);
        return;
    }

    ++currentLoop_;
    if (!infinite_ && currentLoop_ >= loopTarget_) {
        emit progressChanged(currentLoop_, loopTarget_, 0.0);
        stop(true);
        return;
    }

    nextFrameIndex_ = 0;
    emit progressChanged(currentLoop_, loopTarget_, 0.0);
    if (!motionService_->startRevolution(roundTimeSec_)) {
        emit syncLog(QStringLiteral("下一圈定长运动启动失败，已安全结束同步。"));
        stop(false);
    }
}
