#pragma once

#include <QString>

struct AppConfig {
    int axisIndex = 1;
    int pulsePerMotorRev = 1000;
    int gearRatio = 90;
    double pulsePerDeg = 250.0;
    double velMaxPulseMs = 20.0;
    int homeTimeoutMs = 30000;

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

