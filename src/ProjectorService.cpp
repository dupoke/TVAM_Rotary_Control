#include "ProjectorService.h"

#include <QRegularExpression>
#include <QSettings>
#include <QSerialPortInfo>

#include <algorithm>

ProjectorService::ProjectorService(QObject* parent)
    : QObject(parent) {
}

void ProjectorService::applyConfig(const AppConfig& config) {
    config_ = config;
}

QStringList ProjectorService::availablePorts() const {
#ifdef Q_OS_WIN
    // Avoid unstable SetupAPI enumeration path on some driver stacks.
    QSettings serialMap(QStringLiteral("HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM"),
                        QSettings::NativeFormat);
    QStringList names;
    const QStringList keys = serialMap.allKeys();
    for (const QString& key : keys) {
        const QString value = serialMap.value(key).toString().trimmed();
        if (!value.isEmpty()) {
            names.push_back(value.toUpper());
        }
    }
    names.removeDuplicates();
    std::sort(names.begin(), names.end(), [](const QString& a, const QString& b) {
        static const QRegularExpression re(QStringLiteral("^COM(\\d+)$"),
                                           QRegularExpression::CaseInsensitiveOption);
        const auto ma = re.match(a);
        const auto mb = re.match(b);
        if (ma.hasMatch() && mb.hasMatch()) {
            return ma.captured(1).toInt() < mb.captured(1).toInt();
        }
        return a < b;
    });
    return names;
#else
    QStringList names;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& info : ports) {
        names.push_back(info.portName());
    }
    return names;
#endif
}

bool ProjectorService::openPort(const QString& portName, QString* error) {
    if (portName.trimmed().isEmpty()) {
        if (error != nullptr) {
            *error = QStringLiteral("串口名为空。");
        }
        return false;
    }

    if (serial_.isOpen()) {
        serial_.close();
    }

    serial_.setPortName(portName.trimmed());
    serial_.setBaudRate(config_.projectorBaudrate);
    serial_.setDataBits(static_cast<QSerialPort::DataBits>(config_.projectorDataBits));
    serial_.setStopBits(config_.projectorStopBits == 2 ? QSerialPort::TwoStop : QSerialPort::OneStop);

    if (config_.projectorParity == QStringLiteral("odd")) {
        serial_.setParity(QSerialPort::OddParity);
    } else if (config_.projectorParity == QStringLiteral("even")) {
        serial_.setParity(QSerialPort::EvenParity);
    } else {
        serial_.setParity(QSerialPort::NoParity);
    }

    if (config_.projectorFlowControl == QStringLiteral("hardware")) {
        serial_.setFlowControl(QSerialPort::HardwareControl);
    } else if (config_.projectorFlowControl == QStringLiteral("software")) {
        serial_.setFlowControl(QSerialPort::SoftwareControl);
    } else {
        serial_.setFlowControl(QSerialPort::NoFlowControl);
    }

    if (!serial_.open(QIODevice::ReadWrite)) {
        if (error != nullptr) {
            *error = QStringLiteral("打开光机串口失败: %1").arg(serial_.errorString());
        }
        emit projectorLog(QStringLiteral("光机串口打开失败: %1").arg(serial_.errorString()));
        emit portStateChanged(false, QStringLiteral("N/A"));
        return false;
    }

    emit projectorLog(QStringLiteral("光机串口已连接: %1").arg(serial_.portName()));
    emit portStateChanged(true, serial_.portName());
    return true;
}

void ProjectorService::closePort() {
    if (serial_.isOpen()) {
        const QString oldPort = serial_.portName();
        serial_.close();
        emit projectorLog(QStringLiteral("光机串口已关闭: %1").arg(oldPort));
    }
    emit portStateChanged(false, QStringLiteral("N/A"));
}

bool ProjectorService::isPortOpen() const {
    return serial_.isOpen();
}

QString ProjectorService::currentPortName() const {
    if (!serial_.isOpen()) {
        return QStringLiteral("N/A");
    }
    return serial_.portName();
}

bool ProjectorService::setPower(bool on, QString* error) {
    return sendCommand(QStringLiteral("WT+PWRE=%1").arg(on ? 1 : 0), error);
}

bool ProjectorService::setLed(bool on, QString* error) {
    return sendCommand(QStringLiteral("WT+LEDE=%1").arg(on ? 1 : 0), error);
}

bool ProjectorService::setBrightnessPercent(int percent, QString* error) {
    const int normalized = qBound(0, percent, 100);
    const int ledValue = qRound(normalized / 100.0 * 1023.0);
    return sendCommand(QStringLiteral("WT+LEDS=%1").arg(ledValue), error);
}

bool ProjectorService::sendCommand(const QString& cmd, QString* error) {
    QByteArray payload = cmd.toUtf8();
    if (config_.projectorSendWithEnter) {
        // QCOM "Enter" behavior, keep CRLF as default until CR/CRLF is field-verified.
        payload.append("\r\n");
    }

    if (!serial_.isOpen()) {
        emit projectorLog(QStringLiteral("光机串口未连接，模拟发送: %1").arg(cmd));
        return true;
    }

    const qint64 sent = serial_.write(payload);
    if (sent != payload.size() || !serial_.waitForBytesWritten(500)) {
        if (error != nullptr) {
            *error = QStringLiteral("命令发送失败: %1").arg(serial_.errorString());
        }
        emit projectorLog(QStringLiteral("命令发送失败: %1").arg(serial_.errorString()));
        return false;
    }

    emit projectorLog(QStringLiteral("命令发送成功: %1").arg(cmd));
    return true;
}
