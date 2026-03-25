#pragma once

#include <functional>

#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>

class LogService final {
public:
    explicit LogService(QString logDirPath);
    ~LogService();

    bool initialize(QString* error = nullptr);
    void setUiSink(std::function<void(const QString&)> sink);

    void info(const QString& module, const QString& message);
    void warn(const QString& module, const QString& message);
    void error(const QString& module, const QString& message);

private:
    enum class Level {
        Info,
        Warning,
        Error
    };

    void write(Level level, const QString& module, const QString& message);
    static QString levelText(Level level);
    static QString nowText();

    QString logDirPath_;
    QFile logFile_;
    QTextStream logStream_;
    QMutex mutex_;
    std::function<void(const QString&)> uiSink_;
};

