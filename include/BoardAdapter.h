#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

class BoardAdapter final : public QObject {
    Q_OBJECT

public:
    explicit BoardAdapter(QObject* parent = nullptr);

    bool connectBoard(const QString& preferredPort = QString());
    void disconnectBoard();

    bool isConnected() const;
    QString currentPort() const;
    quint32 diRaw() const;
    bool diBit(int index) const;

signals:
    void connectionChanged(bool connected, const QString& portName);
    void diChanged(quint32 rawValue);
    void boardLog(const QString& message);

private:
    void onPollTick();

    bool connected_ = false;
    QString currentPort_;
    quint32 diRaw_ = 0;
    int tickCount_ = 0;
    QTimer pollTimer_;
};

