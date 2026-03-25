#include "BoardAdapter.h"

#include <QRegularExpression>
#include <QSettings>

#include <algorithm>

BoardAdapter::BoardAdapter(QObject* parent)
    : QObject(parent) {
    pollTimer_.setInterval(200);
    connect(&pollTimer_, &QTimer::timeout, this, &BoardAdapter::onPollTick);
}

QStringList BoardAdapter::availablePorts() const {
#ifdef Q_OS_WIN
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
    return {};
#endif
}

bool BoardAdapter::connectBoard(const QString& preferredPort) {
    if (connected_) {
        emit boardLog(QStringLiteral("板卡已连接: %1").arg(currentPort_));
        return true;
    }

    QString selectedPort = preferredPort.trimmed();
    if (selectedPort.isEmpty()) {
        emit boardLog(QStringLiteral("未选择板卡串口，连接失败。"));
        emit connectionChanged(false, QStringLiteral("N/A"));
        return false;
    }

    connected_ = true;
    currentPort_ = selectedPort;
    diRaw_ = 0;
    tickCount_ = 0;
    pollTimer_.start();

    emit connectionChanged(true, currentPort_);
    emit diChanged(diRaw_);
    emit boardLog(QStringLiteral("板卡连接成功(模拟后端)，串口: %1；当前版本尚未接入 COM_GAS_N DLL。")
                      .arg(currentPort_));
    return true;
}

void BoardAdapter::disconnectBoard() {
    if (!connected_) {
        return;
    }
    pollTimer_.stop();
    connected_ = false;
    currentPort_.clear();
    diRaw_ = 0;

    emit diChanged(diRaw_);
    emit connectionChanged(false, QStringLiteral("N/A"));
    emit boardLog(QStringLiteral("板卡已断开连接。"));
}

bool BoardAdapter::isConnected() const {
    return connected_;
}

QString BoardAdapter::currentPort() const {
    return currentPort_;
}

quint32 BoardAdapter::diRaw() const {
    return diRaw_;
}

bool BoardAdapter::diBit(int index) const {
    if (index < 0 || index > 31) {
        return false;
    }
    return (diRaw_ >> index) & 0x1U;
}

void BoardAdapter::onPollTick() {
    if (!connected_) {
        return;
    }
    ++tickCount_;
    // Simulate 4-bit DI status for UI flow debugging.
    diRaw_ = static_cast<quint32>(tickCount_ % 16);
    emit diChanged(diRaw_);
}
