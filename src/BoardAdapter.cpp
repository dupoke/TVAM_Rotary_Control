#include "BoardAdapter.h"

BoardAdapter::BoardAdapter(QObject* parent)
    : QObject(parent) {
    pollTimer_.setInterval(200);
    connect(&pollTimer_, &QTimer::timeout, this, &BoardAdapter::onPollTick);
}

bool BoardAdapter::connectBoard(const QString& preferredPort) {
    if (connected_) {
        emit boardLog(QStringLiteral("板卡已连接: %1").arg(currentPort_));
        return true;
    }

    QString selectedPort = preferredPort.trimmed();
    if (selectedPort.isEmpty()) {
        selectedPort = QStringLiteral("SIM");
    }

    connected_ = true;
    currentPort_ = selectedPort;
    diRaw_ = 0;
    tickCount_ = 0;
    pollTimer_.start();

    emit connectionChanged(true, currentPort_);
    emit diChanged(diRaw_);
    emit boardLog(QStringLiteral("板卡连接成功，端口: %1").arg(currentPort_));
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
