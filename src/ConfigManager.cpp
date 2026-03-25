#include "ConfigManager.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <utility>

ConfigManager::ConfigManager(QString configPath)
    : configPath_(std::move(configPath)) {
}

AppConfig ConfigManager::defaultConfig() {
    return AppConfig{};
}

bool ConfigManager::load(AppConfig& outConfig, QString* error) const {
    AppConfig config = defaultConfig();

    const QFileInfo fileInfo(configPath_);
    if (!fileInfo.exists()) {
        if (!save(config, error)) {
            return false;
        }
        outConfig = config;
        return true;
    }

    QSettings settings(configPath_, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

    config.axisIndex = settings.value("motion/axis_index", config.axisIndex).toInt();
    config.pulsePerMotorRev = settings.value("motion/pulse_per_motor_rev", config.pulsePerMotorRev).toInt();
    config.gearRatio = settings.value("motion/gear_ratio", config.gearRatio).toInt();
    config.pulsePerDeg = settings.value("motion/pulse_per_deg", config.pulsePerDeg).toDouble();
    config.velMaxPulseMs = settings.value("motion/vel_max_pulse_ms", config.velMaxPulseMs).toDouble();
    config.homeTimeoutMs = settings.value("motion/home_timeout_ms", config.homeTimeoutMs).toInt();

    config.projectorBaudrate = settings.value("projector/baudrate", config.projectorBaudrate).toInt();
    config.projectorDataBits = settings.value("projector/data_bits", config.projectorDataBits).toInt();
    config.projectorParity = settings.value("projector/parity", config.projectorParity).toString();
    config.projectorStopBits = settings.value("projector/stop_bits", config.projectorStopBits).toInt();
    config.projectorFlowControl = settings.value("projector/flow_control", config.projectorFlowControl).toString();
    config.projectorSendWithEnter = settings.value("projector/send_with_enter", config.projectorSendWithEnter).toBool();

    normalize(config);
    outConfig = config;
    return true;
}

bool ConfigManager::save(const AppConfig& config, QString* error) const {
    if (!ensureParentDirectory(configPath_, error)) {
        return false;
    }

    AppConfig normalized = config;
    normalize(normalized);

    QSettings settings(configPath_, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

    settings.setValue("meta/version", "1.0");

    settings.setValue("motion/axis_index", normalized.axisIndex);
    settings.setValue("motion/pulse_per_motor_rev", normalized.pulsePerMotorRev);
    settings.setValue("motion/gear_ratio", normalized.gearRatio);
    settings.setValue("motion/pulse_per_deg", normalized.pulsePerDeg);
    settings.setValue("motion/vel_max_pulse_ms", normalized.velMaxPulseMs);
    settings.setValue("motion/home_timeout_ms", normalized.homeTimeoutMs);

    settings.setValue("projector/baudrate", normalized.projectorBaudrate);
    settings.setValue("projector/data_bits", normalized.projectorDataBits);
    settings.setValue("projector/parity", normalized.projectorParity);
    settings.setValue("projector/stop_bits", normalized.projectorStopBits);
    settings.setValue("projector/flow_control", normalized.projectorFlowControl);
    settings.setValue("projector/send_with_enter", normalized.projectorSendWithEnter);

    settings.sync();
    if (settings.status() != QSettings::NoError) {
        if (error != nullptr) {
            *error = QStringLiteral("配置写入失败: %1").arg(configPath_);
        }
        return false;
    }
    return true;
}

void ConfigManager::normalize(AppConfig& config) {
    if (config.axisIndex < 1) {
        config.axisIndex = 1;
    }
    if (config.pulsePerMotorRev <= 0) {
        config.pulsePerMotorRev = 1000;
    }
    if (config.gearRatio <= 0) {
        config.gearRatio = 90;
    }
    if (config.pulsePerDeg <= 0.0) {
        config.pulsePerDeg = 250.0;
    }
    if (config.velMaxPulseMs <= 0.0) {
        config.velMaxPulseMs = 20.0;
    }
    if (config.homeTimeoutMs < 1000) {
        config.homeTimeoutMs = 30000;
    }

    if (config.projectorBaudrate <= 0) {
        config.projectorBaudrate = 115200;
    }
    if (config.projectorDataBits < 5 || config.projectorDataBits > 8) {
        config.projectorDataBits = 8;
    }
    if (config.projectorStopBits != 1 && config.projectorStopBits != 2) {
        config.projectorStopBits = 1;
    }

    config.projectorParity = config.projectorParity.trimmed().toLower();
    if (config.projectorParity != "none" &&
        config.projectorParity != "odd" &&
        config.projectorParity != "even") {
        config.projectorParity = "none";
    }

    config.projectorFlowControl = config.projectorFlowControl.trimmed().toLower();
    if (config.projectorFlowControl != "none" &&
        config.projectorFlowControl != "software" &&
        config.projectorFlowControl != "hardware") {
        config.projectorFlowControl = "none";
    }
}

bool ConfigManager::ensureParentDirectory(const QString& filePath, QString* error) {
    const QFileInfo info(filePath);
    QDir dir = info.dir();
    if (dir.exists()) {
        return true;
    }
    if (dir.mkpath(".")) {
        return true;
    }
    if (error != nullptr) {
        *error = QStringLiteral("创建配置目录失败: %1").arg(dir.path());
    }
    return false;
}
