#include "ImageSequenceService.h"

#include <QDir>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>

namespace {

double normalizeAngle(double angleDeg) {
    double normalized = std::fmod(angleDeg, 360.0);
    if (normalized < 0.0) {
        normalized += 360.0;
    }
    if (std::abs(normalized - 360.0) < 1e-6) {
        normalized = 0.0;
    }
    return normalized;
}

}  // namespace

SequenceCheckResult ImageSequenceService::checkFolder(const QString& folderPath) const {
    SequenceCheckResult result;

    const QDir dir(folderPath);
    if (!dir.exists()) {
        result.message = QStringLiteral("目录不存在。");
        return result;
    }

    const QStringList filters = {
        QStringLiteral("*.png"),
        QStringLiteral("*.jpg"),
        QStringLiteral("*.jpeg"),
        QStringLiteral("*.bmp"),
        QStringLiteral("*.tif"),
        QStringLiteral("*.tiff"),
        QStringLiteral("*.webp")
    };
    const QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::NoSymLinks, QDir::Name);
    if (files.isEmpty()) {
        result.message = QStringLiteral("目录中未找到图片文件。");
        return result;
    }

    const QRegularExpression pattern(QStringLiteral(".*?(\\d+(?:\\.\\d+)?)deg.*"),
                                     QRegularExpression::CaseInsensitiveOption);
    for (const QFileInfo& info : files) {
        const auto match = pattern.match(info.fileName());
        if (!match.hasMatch()) {
            ++result.ignoredFileCount;
            continue;
        }

        bool ok = false;
        const double angle = match.captured(1).toDouble(&ok);
        if (!ok) {
            ++result.ignoredFileCount;
            continue;
        }

        ImageFrame frame;
        frame.angleDeg = normalizeAngle(angle);
        frame.filePath = info.absoluteFilePath();
        frame.fileName = info.fileName();
        result.frames.push_back(frame);
    }

    if (result.frames.isEmpty()) {
        result.message = QStringLiteral("未识别到包含角度编号的图片（例如 0deg、2.0deg）。");
        return result;
    }

    std::sort(result.frames.begin(), result.frames.end(),
              [](const ImageFrame& a, const ImageFrame& b) { return a.angleDeg < b.angleDeg; });

    for (int i = 1; i < result.frames.size(); ++i) {
        if (std::abs(result.frames[i].angleDeg - result.frames[i - 1].angleDeg) < 1e-6) {
            result.message = QStringLiteral("检测到重复角度: %1°。").arg(result.frames[i].angleDeg, 0, 'f', 6);
            result.ok = false;
            return result;
        }
    }

    if (result.frames.size() < 2) {
        result.message = QStringLiteral("有效图片数量不足，至少需要2张。");
        return result;
    }

    QVector<double> diffs;
    diffs.reserve(result.frames.size());
    for (int i = 1; i < result.frames.size(); ++i) {
        diffs.push_back(result.frames[i].angleDeg - result.frames[i - 1].angleDeg);
    }
    diffs.push_back(360.0 - result.frames.back().angleDeg + result.frames.front().angleDeg);
    std::sort(diffs.begin(), diffs.end());
    result.suggestedStepDeg = diffs[diffs.size() / 2];

    result.ok = true;
    result.message = QStringLiteral("序列检查通过: 有效%1张，忽略%2张，建议步距约%3°。")
                         .arg(result.frames.size())
                         .arg(result.ignoredFileCount)
                         .arg(result.suggestedStepDeg, 0, 'f', 3);
    return result;
}

