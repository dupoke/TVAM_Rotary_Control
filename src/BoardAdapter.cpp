#include "BoardAdapter.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>

#include <algorithm>

namespace {

constexpr short kOpenModeSerial = 1;
constexpr short kDiTypeGpi = 4;
constexpr long kAxisRunningBit = 0x00000400;

template <typename FnType>
FnType resolveFunction(QLibrary& lib, const QStringList& names) {
    for (const QString& name : names) {
        const QByteArray symbol = name.toLatin1();
        if (void* ptr = lib.resolve(symbol.constData())) {
            return reinterpret_cast<FnType>(ptr);
        }
    }
    return nullptr;
}

}  // namespace

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
    normalizePortName(selectedPort);

    if (!loadApi()) {
        emit boardLog(QStringLiteral("板卡连接失败: 未加载 GAS.dll/COM_GAS_N.dll。"));
        emit connectionChanged(false, QStringLiteral("N/A"));
        return false;
    }

    if (gaClose_ != nullptr) {
        gaClose_();
    }

    int rc = -9999;
    bool opened = false;

    auto tryOpen = [&](const QString& port, const QString& tag) {
        QByteArray arg = port.toLocal8Bit();
        rc = gaOpen_(kOpenModeSerial, arg.data());
        if (rc == 0) {
            emit boardLog(QStringLiteral("GA_Open 成功，模式: %1，参数: \"%2\"。").arg(tag, port));
            opened = true;
            return;
        }
        emit boardLog(QStringLiteral("GA_Open 失败(%1): %2，模式: %3，参数: \"%4\"。")
                          .arg(rc)
                          .arg(errorText(rc))
                          .arg(tag, port));
    };

    tryOpen(selectedPort, QStringLiteral("手动串口"));
    if (!opened) {
        tryOpen(QString(), QStringLiteral("自动搜索"));
    }
    if (!opened) {
        emit connectionChanged(false, QStringLiteral("N/A"));
        return false;
    }

    rc = gaReset_();
    if (rc != 0) {
        // Keep behavior aligned with vendor demo: reset failure should not always block open.
        emit boardLog(QStringLiteral("GA_Reset 返回(%1): %2；继续保持已连接状态。")
                          .arg(rc)
                          .arg(errorText(rc)));
    }

    connected_ = true;
    currentPort_ = selectedPort;
    diRaw_ = 0;
    diReadErrorCount_ = 0;
    pollTimer_.start();

    emit connectionChanged(true, currentPort_);
    emit diChanged(diRaw_);
    emit boardLog(QStringLiteral("板卡连接成功，串口: %1").arg(currentPort_));
    return true;
}

void BoardAdapter::disconnectBoard() {
    if (!connected_) {
        return;
    }
    pollTimer_.stop();
    if (gaClose_ != nullptr) {
        const int rc = gaClose_();
        if (rc != 0) {
            emit boardLog(QStringLiteral("GA_Close 返回(%1): %2").arg(rc).arg(errorText(rc)));
        }
    }
    connected_ = false;
    currentPort_.clear();
    diRaw_ = 0;
    diReadErrorCount_ = 0;

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

bool BoardAdapter::jogStart(int axisIndex, int direction, double velPulsePerMs, double accPulsePerMs2,
                            double decPulsePerMs2, QString* error) {
    if (!ensureMotionApi(error)) {
        return false;
    }
    if (!validateAxisIndex(axisIndex)) {
        if (error != nullptr) {
            *error = QStringLiteral("轴号无效: %1").arg(axisIndex);
        }
        return false;
    }
    if (direction == 0) {
        if (error != nullptr) {
            *error = QStringLiteral("点动方向无效。");
        }
        return false;
    }

    int rc = gaAxisOn_(static_cast<short>(axisIndex));
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_AxisOn 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    rc = gaPrfJog_(static_cast<short>(axisIndex));
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_PrfJog 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }

    JogPrm prm{};
    prm.dAcc = qMax(0.001, accPulsePerMs2);
    prm.dDec = qMax(0.001, decPulsePerMs2);
    prm.dSmooth = 0.0;
    rc = gaSetJogPrm_(static_cast<short>(axisIndex), &prm);
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_SetJogPrm 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }

    const double signedVel = qMax(0.001, velPulsePerMs) * (direction > 0 ? 1.0 : -1.0);
    rc = gaSetVel_(static_cast<short>(axisIndex), signedVel);
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_SetVel 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }

    rc = gaUpdate_(axisMask(axisIndex));
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_Update 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    return true;
}

bool BoardAdapter::moveToPulse(int axisIndex, qint64 pulse, double velPulsePerMs, double accPulsePerMs2,
                               double decPulsePerMs2, QString* error) {
    if (!ensureMotionApi(error)) {
        return false;
    }
    if (!validateAxisIndex(axisIndex)) {
        if (error != nullptr) {
            *error = QStringLiteral("轴号无效: %1").arg(axisIndex);
        }
        return false;
    }

    int rc = gaAxisOn_(static_cast<short>(axisIndex));
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_AxisOn 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    rc = gaPrfTrap_(static_cast<short>(axisIndex));
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_PrfTrap 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }

    TrapPrm prm{};
    prm.acc = qMax(0.001, accPulsePerMs2);
    prm.dec = qMax(0.001, decPulsePerMs2);
    prm.velStart = 0.0;
    prm.smoothTime = 0;
    rc = gaSetTrapPrm_(static_cast<short>(axisIndex), &prm);
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_SetTrapPrm 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }

    const long target = static_cast<long>(pulse);
    rc = gaSetPos_(static_cast<short>(axisIndex), target);
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_SetPos 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    rc = gaSetVel_(static_cast<short>(axisIndex), qMax(0.001, velPulsePerMs));
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_SetVel 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    rc = gaUpdate_(axisMask(axisIndex));
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_Update 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    return true;
}

bool BoardAdapter::stopAxis(int axisIndex, bool estop, QString* error) {
    if (!ensureMotionApi(error)) {
        return false;
    }
    if (!validateAxisIndex(axisIndex)) {
        if (error != nullptr) {
            *error = QStringLiteral("轴号无效: %1").arg(axisIndex);
        }
        return false;
    }

    const long mask = axisMask(axisIndex);
    const long option = estop ? 0xFFFF : 0;
    const int rc = gaStop_(mask, option);
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_Stop 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    return true;
}

bool BoardAdapter::getProfilePositionPulse(int axisIndex, double& outPulse, QString* error) {
    if (!ensureMotionApi(error)) {
        return false;
    }
    if (!validateAxisIndex(axisIndex)) {
        if (error != nullptr) {
            *error = QStringLiteral("轴号无效: %1").arg(axisIndex);
        }
        return false;
    }

    unsigned long clock = 0;
    double pos = 0.0;
    const int rc = gaGetPrfPos_(static_cast<short>(axisIndex), &pos, 1, &clock);
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_GetPrfPos 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    outPulse = pos;
    return true;
}

bool BoardAdapter::getAxisStatus(int axisIndex, long& outStatus, QString* error) {
    if (!ensureMotionApi(error)) {
        return false;
    }
    if (!validateAxisIndex(axisIndex)) {
        if (error != nullptr) {
            *error = QStringLiteral("轴号无效: %1").arg(axisIndex);
        }
        return false;
    }

    unsigned long clock = 0;
    long sts = 0;
    const int rc = gaGetSts_(static_cast<short>(axisIndex), &sts, 1, &clock);
    if (rc != 0) {
        if (error != nullptr) {
            *error = QStringLiteral("GA_GetSts 失败(%1): %2").arg(rc).arg(errorText(rc));
        }
        return false;
    }
    outStatus = sts;
    return true;
}

void BoardAdapter::onPollTick() {
    if (!connected_) {
        return;
    }

    if (gaGetDiRaw_ == nullptr) {
        return;
    }

    long raw = 0;
    const int rc = gaGetDiRaw_(kDiTypeGpi, &raw);
    if (rc != 0) {
        ++diReadErrorCount_;
        if (diReadErrorCount_ == 1 || (diReadErrorCount_ % 10) == 0) {
            emit boardLog(QStringLiteral("GA_GetDiRaw 失败(%1): %2").arg(rc).arg(errorText(rc)));
        }
        return;
    }

    diReadErrorCount_ = 0;
    const quint32 normalized = static_cast<quint32>(raw);
    if (normalized != diRaw_) {
        diRaw_ = normalized;
        emit diChanged(diRaw_);
    }
}

bool BoardAdapter::loadApi() {
    if (gaOpen_ != nullptr && gaReset_ != nullptr && gaClose_ != nullptr && gaGetDiRaw_ != nullptr) {
        return true;
    }

    unloadApi();
    QStringList tried;
    QStringList loadErrors;
    for (const QString& candidate : candidateLibraryPaths()) {
        tried.push_back(QDir::toNativeSeparators(candidate));
        library_.setFileName(candidate);
        if (!library_.load()) {
            if (loadErrors.size() < 5) {
                loadErrors.push_back(QStringLiteral("%1 => %2")
                                         .arg(QDir::toNativeSeparators(candidate), library_.errorString()));
            }
            continue;
        }

        gaOpen_ = resolveFunction<GAOpenFn>(library_, {QStringLiteral("GA_Open"), QStringLiteral("_GA_Open@8")});
        gaReset_ = resolveFunction<GAResetFn>(library_, {QStringLiteral("GA_Reset"), QStringLiteral("_GA_Reset@0")});
        gaClose_ = resolveFunction<GACloseFn>(library_, {QStringLiteral("GA_Close"), QStringLiteral("_GA_Close@0")});
        gaGetDiRaw_ = resolveFunction<GAGetDiRawFn>(library_, {QStringLiteral("GA_GetDiRaw"), QStringLiteral("_GA_GetDiRaw@8")});
        gaAxisOn_ = resolveFunction<GAAxisOnFn>(library_, {QStringLiteral("GA_AxisOn"), QStringLiteral("_GA_AxisOn@4")});
        gaPrfTrap_ = resolveFunction<GAPrfTrapFn>(library_, {QStringLiteral("GA_PrfTrap"), QStringLiteral("_GA_PrfTrap@4")});
        gaSetTrapPrm_ = resolveFunction<GASetTrapPrmFn>(library_, {QStringLiteral("GA_SetTrapPrm"), QStringLiteral("_GA_SetTrapPrm@8")});
        gaPrfJog_ = resolveFunction<GAPrfJogFn>(library_, {QStringLiteral("GA_PrfJog"), QStringLiteral("_GA_PrfJog@4")});
        gaSetJogPrm_ = resolveFunction<GASetJogPrmFn>(library_, {QStringLiteral("GA_SetJogPrm"), QStringLiteral("_GA_SetJogPrm@8")});
        gaSetPos_ = resolveFunction<GASetPosFn>(library_, {QStringLiteral("GA_SetPos"), QStringLiteral("_GA_SetPos@8")});
        gaSetVel_ = resolveFunction<GASetVelFn>(library_, {QStringLiteral("GA_SetVel"), QStringLiteral("_GA_SetVel@12")});
        gaUpdate_ = resolveFunction<GAUpdateFn>(library_, {QStringLiteral("GA_Update"), QStringLiteral("_GA_Update@4")});
        gaStop_ = resolveFunction<GAStopFn>(library_, {QStringLiteral("GA_Stop"), QStringLiteral("_GA_Stop@8")});
        gaGetPrfPos_ = resolveFunction<GAGetPrfPosFn>(library_, {QStringLiteral("GA_GetPrfPos"), QStringLiteral("_GA_GetPrfPos@16")});
        gaGetSts_ = resolveFunction<GAGetStsFn>(library_, {QStringLiteral("GA_GetSts"), QStringLiteral("_GA_GetSts@16")});
        if (gaOpen_ != nullptr && gaReset_ != nullptr && gaClose_ != nullptr && gaGetDiRaw_ != nullptr) {
            emit boardLog(QStringLiteral("已加载板卡库: %1").arg(QDir::toNativeSeparators(library_.fileName())));
            return true;
        }

        emit boardLog(QStringLiteral("已加载 %1 但缺少必要符号，继续尝试其他路径。")
                          .arg(QDir::toNativeSeparators(library_.fileName())));
        unloadApi();
    }

    emit boardLog(QStringLiteral("未找到可用的 GAS.dll/COM_GAS_N.dll，请放到程序目录或 third_party/board/x64，或设置环境变量 COM_GAS_N_DLL。"));
    emit boardLog(QStringLiteral("已尝试路径: %1").arg(tried.join(QStringLiteral(" | "))));
    if (!loadErrors.isEmpty()) {
        emit boardLog(QStringLiteral("加载错误示例: %1").arg(loadErrors.join(QStringLiteral(" || "))));
    }
    return false;
}

void BoardAdapter::unloadApi() {
    gaOpen_ = nullptr;
    gaReset_ = nullptr;
    gaClose_ = nullptr;
    gaGetDiRaw_ = nullptr;
    gaAxisOn_ = nullptr;
    gaPrfTrap_ = nullptr;
    gaSetTrapPrm_ = nullptr;
    gaPrfJog_ = nullptr;
    gaSetJogPrm_ = nullptr;
    gaSetPos_ = nullptr;
    gaSetVel_ = nullptr;
    gaUpdate_ = nullptr;
    gaStop_ = nullptr;
    gaGetPrfPos_ = nullptr;
    gaGetSts_ = nullptr;
    if (library_.isLoaded()) {
        library_.unload();
    }
}

QString BoardAdapter::errorText(int code) {
    switch (code) {
    case 0:
        return QStringLiteral("执行成功");
    case 1:
        return QStringLiteral("执行失败");
    case 2:
        return QStringLiteral("版本不支持该接口");
    case 7:
        return QStringLiteral("参数错误");
    case -1:
        return QStringLiteral("通讯失败");
    case -6:
        return QStringLiteral("打开控制器失败");
    case -7:
        return QStringLiteral("控制器无响应");
    default:
        return QStringLiteral("未知错误");
    }
}

QStringList BoardAdapter::candidateLibraryPaths() {
    QStringList candidates;

    const QString envPath = QString::fromLocal8Bit(qgetenv("COM_GAS_N_DLL")).trimmed();
    if (!envPath.isEmpty()) {
        candidates.push_back(envPath);
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("GAS.dll")));
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("gas.dll")));
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("COM_GAS_N.dll")));
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("com_gas_n.dll")));
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("..\\GAS.dll")));
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("..\\gas.dll")));
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("..\\COM_GAS_N.dll")));
    candidates.push_back(QDir(appDir).filePath(QStringLiteral("..\\..\\COM_GAS_N.dll")));

    const QString cwd = QDir::currentPath();
    candidates.push_back(QDir(cwd).filePath(QStringLiteral("GAS.dll")));
    candidates.push_back(QDir(cwd).filePath(QStringLiteral("gas.dll")));
    candidates.push_back(QDir(cwd).filePath(QStringLiteral("COM_GAS_N.dll")));
    candidates.push_back(QDir(cwd).filePath(QStringLiteral("third_party\\board\\x64\\GAS.dll")));

    candidates.push_back(QStringLiteral("GAS"));
    candidates.push_back(QStringLiteral("COM_GAS_N"));

    QStringList unique;
    for (const QString& path : candidates) {
        const QString normalized = QDir::fromNativeSeparators(path);
        if (!normalized.trimmed().isEmpty() && !unique.contains(normalized)) {
            unique.push_back(normalized);
        }
    }
    return unique;
}

void BoardAdapter::normalizePortName(QString& port) {
    port = port.trimmed().toUpper();
    if (!port.startsWith(QStringLiteral("COM"))) {
        port = QStringLiteral("COM%1").arg(port);
    }
}

bool BoardAdapter::validateAxisIndex(int axisIndex) {
    return axisIndex >= 1 && axisIndex <= 16;
}

long BoardAdapter::axisMask(int axisIndex) {
    return 1L << (axisIndex - 1);
}

bool BoardAdapter::ensureMotionApi(QString* error) {
    if (!connected_) {
        if (error != nullptr) {
            *error = QStringLiteral("板卡未连接。");
        }
        return false;
    }
    if (gaAxisOn_ == nullptr || gaPrfTrap_ == nullptr || gaSetTrapPrm_ == nullptr ||
        gaPrfJog_ == nullptr || gaSetJogPrm_ == nullptr || gaSetPos_ == nullptr ||
        gaSetVel_ == nullptr || gaUpdate_ == nullptr || gaStop_ == nullptr ||
        gaGetPrfPos_ == nullptr || gaGetSts_ == nullptr) {
        if (error != nullptr) {
            *error = QStringLiteral("板卡运动接口未就绪。");
        }
        return false;
    }
    return true;
}
