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

    if (!motionService_->startContinuous(roundTimeSec)) {
        if (error != nullptr) {
            *error = QStringLiteral("启动连续旋转失败。");
        }
        return false;
    }

    running_ = true;
    infinite_ = infinite;
    loopTarget_ = infinite ? 0 : loopCount;
    currentLoop_ = 0;
    nextFrameIndex_ = 0;
    startTotalDeg_ = motionService_->currentTotalDeg();

    emit stateChanged(QStringLiteral("SYNC_RUNNING"));
    emit progressChanged(currentLoop_, loopTarget_, 0.0);
    return true;
}

void SyncPlaybackService::stop(bool completed) {
    if (!running_) {
        return;
    }

    running_ = false;
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

    if (nextFrameIndex_ >= frames_.size()) {
        nextFrameIndex_ = 0;
        ++currentLoop_;
        if (!infinite_ && currentLoop_ >= loopTarget_) {
            stop(true);
            return;
        }
    }

    Q_UNUSED(cycleDeg);
    emit progressChanged(currentLoop_, loopTarget_, normalizeRelativeDeg(relativeDeg));
}
