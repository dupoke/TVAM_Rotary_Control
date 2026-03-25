#pragma once

#include <QObject>
#include <QLibrary>
#include <QStringList>
#include <QString>
#include <QTimer>
#include <QtGlobal>

class BoardAdapter final : public QObject {
    Q_OBJECT

public:
    explicit BoardAdapter(QObject* parent = nullptr);

    QStringList availablePorts() const;
    bool connectBoard(const QString& preferredPort = QString());
    void disconnectBoard();

    bool isConnected() const;
    QString currentPort() const;
    quint32 diRaw() const;
    bool diBit(int index) const;

    bool jogStart(int axisIndex, int direction, double velPulsePerMs, double accPulsePerMs2,
                  double decPulsePerMs2, QString* error = nullptr);
    bool moveToPulse(int axisIndex, qint64 pulse, double velPulsePerMs, double accPulsePerMs2,
                     double decPulsePerMs2, QString* error = nullptr);
    bool stopAxis(int axisIndex, bool estop, QString* error = nullptr);
    bool getProfilePositionPulse(int axisIndex, double& outPulse, QString* error = nullptr);
    bool getAxisStatus(int axisIndex, long& outStatus, QString* error = nullptr);

signals:
    void connectionChanged(bool connected, const QString& portName);
    void diChanged(quint32 rawValue);
    void boardLog(const QString& message);

private:
    struct TrapPrm {
        double acc;
        double dec;
        double velStart;
        short smoothTime;
    };

    struct JogPrm {
        double dAcc;
        double dDec;
        double dSmooth;
    };

    using GAOpenFn = int (*)(short, char*);
    using GAResetFn = int (*)();
    using GACloseFn = int (*)();
    using GAGetDiRawFn = int (*)(short, long*);
    using GAAxisOnFn = int (*)(short);
    using GAPrfTrapFn = int (*)(short);
    using GASetTrapPrmFn = int (*)(short, TrapPrm*);
    using GAPrfJogFn = int (*)(short);
    using GASetJogPrmFn = int (*)(short, JogPrm*);
    using GASetPosFn = int (*)(short, long);
    using GASetVelFn = int (*)(short, double);
    using GAUpdateFn = int (*)(long);
    using GAStopFn = int (*)(long, long);
    using GAGetPrfPosFn = int (*)(short, double*, short, unsigned long*);
    using GAGetStsFn = int (*)(short, long*, short, unsigned long*);

    bool loadApi();
    void unloadApi();
    static QString errorText(int code);
    static QStringList candidateLibraryPaths();
    static void normalizePortName(QString& port);
    static bool validateAxisIndex(int axisIndex);
    static long axisMask(int axisIndex);
    bool ensureMotionApi(QString* error);

    void onPollTick();

    bool connected_ = false;
    QString currentPort_;
    quint32 diRaw_ = 0;
    int diReadErrorCount_ = 0;
    QTimer pollTimer_;

    QLibrary library_;
    GAOpenFn gaOpen_ = nullptr;
    GAResetFn gaReset_ = nullptr;
    GACloseFn gaClose_ = nullptr;
    GAGetDiRawFn gaGetDiRaw_ = nullptr;
    GAAxisOnFn gaAxisOn_ = nullptr;
    GAPrfTrapFn gaPrfTrap_ = nullptr;
    GASetTrapPrmFn gaSetTrapPrm_ = nullptr;
    GAPrfJogFn gaPrfJog_ = nullptr;
    GASetJogPrmFn gaSetJogPrm_ = nullptr;
    GASetPosFn gaSetPos_ = nullptr;
    GASetVelFn gaSetVel_ = nullptr;
    GAUpdateFn gaUpdate_ = nullptr;
    GAStopFn gaStop_ = nullptr;
    GAGetPrfPosFn gaGetPrfPos_ = nullptr;
    GAGetStsFn gaGetSts_ = nullptr;
};
