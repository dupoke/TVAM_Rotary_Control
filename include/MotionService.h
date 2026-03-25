#pragma once

#include <QObject>
#include <QTimer>

class MotionService final : public QObject {
    Q_OBJECT

public:
    explicit MotionService(QObject* parent = nullptr);

    void setPulsePerDeg(double value);
    void setMaxVelocityPulsePerMs(double value);

    bool jogStart(int direction);
    bool moveToAbsDeg(double targetDeg);
    bool moveByRelDeg(double deltaDeg);
    bool startContinuous(double roundTimeSec);
    void stop(bool estop);

    qint64 currentPulse() const;
    double currentTotalDeg() const;
    double currentCycleDeg() const;
    QString stateText() const;

signals:
    void positionChanged(qint64 pulse, double totalDeg, double cycleDeg);
    void stateChanged(const QString& state);
    void motionFinished();
    void motionLog(const QString& message);
    void motionError(const QString& message);

private:
    enum class Mode {
        Idle,
        Jogging,
        Positioning,
        Continuous
    };

    static double normalizeDeg(double value);
    void setMode(Mode mode);
    void onTick();

    QTimer tickTimer_;
    Mode mode_ = Mode::Idle;

    qint64 currentPulse_ = 0;
    qint64 targetPulse_ = 0;

    double pulsePerDeg_ = 250.0;
    double maxVelPulsePerSec_ = 20000.0;
    double jogVelPulsePerSec_ = 2000.0;

    int jogDirection_ = 1;
    double continuousPulsePerSec_ = 0.0;
};

