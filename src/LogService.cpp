#include "LogService.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <utility>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

LogService::LogService(QString logDirPath)
    : logDirPath_(std::move(logDirPath)) {
}

LogService::~LogService() {
    if (logFile_.isOpen()) {
        logFile_.flush();
        logFile_.close();
    }
}

bool LogService::initialize(QString* error) {
    QDir dir(logDirPath_);
    if (!dir.exists() && !dir.mkpath(".")) {
        if (error != nullptr) {
            *error = QStringLiteral("创建日志目录失败: %1").arg(logDirPath_);
        }
        return false;
    }

    const QString logFileName = QStringLiteral("app_%1.log")
                                    .arg(QDate::currentDate().toString(QStringLiteral("yyyyMMdd")));
    const QString logFilePath = dir.filePath(logFileName);
    logFile_.setFileName(logFilePath);
    if (!logFile_.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        if (error != nullptr) {
            *error = QStringLiteral("打开日志文件失败: %1").arg(logFilePath);
        }
        return false;
    }

    logStream_.setDevice(&logFile_);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    logStream_.setEncoding(QStringConverter::Utf8);
#else
    logStream_.setCodec("UTF-8");
#endif
    return true;
}

void LogService::setUiSink(std::function<void(const QString&)> sink) {
    QMutexLocker locker(&mutex_);
    uiSink_ = std::move(sink);
}

void LogService::info(const QString& module, const QString& message) {
    write(Level::Info, module, message);
}

void LogService::warn(const QString& module, const QString& message) {
    write(Level::Warning, module, message);
}

void LogService::error(const QString& module, const QString& message) {
    write(Level::Error, module, message);
}

void LogService::write(Level level, const QString& module, const QString& message) {
    const QString line = QStringLiteral("[%1][%2][%3] %4")
                             .arg(nowText(), levelText(level), module, message);

    QMutexLocker locker(&mutex_);
    if (logFile_.isOpen()) {
        logStream_ << line << '\n';
        logStream_.flush();
    }
    if (uiSink_) {
        uiSink_(line);
    }
}

QString LogService::levelText(Level level) {
    switch (level) {
    case Level::Info:
        return QStringLiteral("信息");
    case Level::Warning:
        return QStringLiteral("警告");
    case Level::Error:
        return QStringLiteral("错误");
    default:
        return QStringLiteral("信息");
    }
}

QString LogService::nowText() {
    return QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
}
