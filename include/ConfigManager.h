#pragma once

#include <QString>

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
    int fineHomeDiIndex = 1;
    bool fineHomeActiveLow = false;
    int fineHomeDirection = -1;
    double fineHomeMaxTravelDeg = 360.0;
    int fineHomeTimeoutMs = 60000;
    int zLaserDiIndex = 0;
    bool zLaserActiveLow = true;
    int laserStableMs = 120;

    int projectorBaudrate = 115200;
    int projectorDataBits = 8;
    QString projectorParity = "none";
    int projectorStopBits = 1;
    QString projectorFlowControl = "none";
    bool projectorSendWithEnter = true;
};

class ConfigManager final {
public:
    explicit ConfigManager(QString configPath);

    bool load(AppConfig& outConfig, QString* error = nullptr) const;
    bool save(const AppConfig& config, QString* error = nullptr) const;

    static AppConfig defaultConfig();

private:
    static void normalize(AppConfig& config);
    static bool ensureParentDirectory(const QString& filePath, QString* error);

    QString configPath_;
};
