#pragma once

#include <QObject>
#include <QLibrary>
#include <QStringList>
#include <QString>
#include <QTimer>

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

signals:
    void connectionChanged(bool connected, const QString& portName);
    void diChanged(quint32 rawValue);
    void boardLog(const QString& message);

private:
    using GAOpenFn = int (*)(short, char*);
    using GAResetFn = int (*)();
    using GACloseFn = int (*)();
    using GAGetDiRawFn = int (*)(short, long*);

    bool loadApi();
    void unloadApi();
    static QString errorText(int code);
    static QStringList candidateLibraryPaths();
    static void normalizePortName(QString& port);

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
};
