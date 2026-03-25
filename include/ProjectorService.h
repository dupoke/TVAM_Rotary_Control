#pragma once

#include <QObject>
#include <QSerialPort>
#include <QStringList>

#include "ConfigManager.h"

class ProjectorService final : public QObject {
    Q_OBJECT

public:
    explicit ProjectorService(QObject* parent = nullptr);

    void applyConfig(const AppConfig& config);
    QStringList availablePorts() const;

    bool openPort(const QString& portName, QString* error = nullptr);
    void closePort();
    bool isPortOpen() const;
    QString currentPortName() const;

    bool setPower(bool on, QString* error = nullptr);
    bool setLed(bool on, QString* error = nullptr);
    bool setBrightnessPercent(int percent, QString* error = nullptr);

signals:
    void portStateChanged(bool opened, const QString& portName);
    void projectorLog(const QString& message);

private:
    bool sendCommand(const QString& cmd, QString* error);

    QSerialPort serial_;
    AppConfig config_;
};

