#pragma once

#include <QObject>
#include <QVector>

#include "ImageSequenceService.h"

class MotionService;

class SyncPlaybackService final : public QObject {
    Q_OBJECT

public:
    explicit SyncPlaybackService(MotionService* motionService, QObject* parent = nullptr);

    void setFrames(QVector<ImageFrame> frames);
    bool start(double roundTimeSec, int loopCount, bool infinite, QString* error = nullptr);
    void stop(bool completed = false);

    bool isRunning() const;
    int currentLoop() const;
    int loopTarget() const;

signals:
    void stateChanged(const QString& stateText);
    void progressChanged(int loopIndex, int loopTarget, double cycleDeg);
    void frameChanged(int frameIndex, int loopIndex, const QString& filePath, double angleDeg);
    void syncFinished(bool completed);
    void syncLog(const QString& message);

private:
    void onMotionPositionChanged(qint64 pulse, double totalDeg, double cycleDeg);

    MotionService* motionService_ = nullptr;
    QVector<ImageFrame> frames_;

    bool running_ = false;
    bool infinite_ = false;
    int loopTarget_ = 1;
    int currentLoop_ = 0;
    int nextFrameIndex_ = 0;
    double startTotalDeg_ = 0.0;
};
