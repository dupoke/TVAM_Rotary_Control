#pragma once

#include <QString>
#include <QVariantMap>

struct AppConfig {
    int axisIndex = 1;
    int pulsePerMotorRev = 1000;
    int gearRatio = 180;
    double pulsePerDeg = 500.0;
    double velMaxPulseMs = 20.0;
    int homeTimeoutMs = 30000;
    int coarseHomeDiIndex = 1;
    int coarseHomeDirection = -1;
    double coarseHomeMaxTurns = 1.0;
    int fineHomeDiIndex = 0;
    bool fineHomeActiveLow = true;
    int fineHomeDirection = -1;
    double fineHomeSpeedDegPerSec = 0.2;
    int fineHomePollIntervalMs = 5;
    double fineHomeMaxTravelDeg = 360.0;
    int fineHomeTimeoutMs = 0;
    int zLaserDiIndex = 0;
    bool zLaserActiveLow = true;
    int laserStableMs = 120;

    int projectorBaudrate = 115200;
    int projectorDataBits = 8;
    QString projectorParity = "none";
    int projectorStopBits = 1;
    QString projectorFlowControl = "none";
    bool projectorSendWithEnter = true;

    // 路径记忆
    QString lastImageFolderPath;
    QString lastCameraSavePath;
};

class ConfigManager final {
public:
    explicit ConfigManager(QString configPath);

    bool load(AppConfig& outConfig, QString* error = nullptr) const;
    bool save(const AppConfig& config, QString* error = nullptr) const;
    QVariantMap loadCameraProfile(const QString& deviceId) const;
    bool saveCameraProfile(const QString& deviceId, const QVariantMap& profile,
                           QString* error = nullptr) const;

    static AppConfig defaultConfig();

private:
    static void normalize(AppConfig& config);
    static bool ensureParentDirectory(const QString& filePath, QString* error);

    QString configPath_;
};
