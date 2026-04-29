#include "ImageSequenceService.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>

namespace {

const QStringList kSupportedImageFilters = {
    QStringLiteral("*.png"),
    QStringLiteral("*.jpg"),
    QStringLiteral("*.jpeg"),
    QStringLiteral("*.bmp"),
    QStringLiteral("*.tif"),
    QStringLiteral("*.tiff"),
    QStringLiteral("*.webp")
};

QString formatFrameIndex(int index) {
    return QStringLiteral("%1").arg(index, 4, 10, QLatin1Char('0'));
}

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

double computeSuggestedStepDeg(const QVector<ImageFrame>& frames) {
    QVector<double> diffs;
    diffs.reserve(frames.size());
    for (int i = 1; i < frames.size(); ++i) {
        diffs.push_back(frames[i].angleDeg - frames[i - 1].angleDeg);
    }
    diffs.push_back(360.0 - frames.back().angleDeg + frames.front().angleDeg);
    std::sort(diffs.begin(), diffs.end());
    return diffs[diffs.size() / 2];
}

}  // namespace

SequenceCheckResult ImageSequenceService::checkFolder(const QString& folderPath) const {
    SequenceCheckResult result;

    const QDir dir(folderPath);
    if (!dir.exists()) {
        result.message = QStringLiteral("目录不存在。");
        return result;
    }

    const QFileInfoList files =
        dir.entryInfoList(kSupportedImageFilters, QDir::Files | QDir::NoSymLinks, QDir::Name);
    const QFileInfoList exrFiles =
        dir.entryInfoList({QStringLiteral("*.exr")}, QDir::Files | QDir::NoSymLinks, QDir::Name);
    if (files.isEmpty()) {
        result.message = exrFiles.isEmpty()
                             ? QStringLiteral("目录中未找到图片文件。")
                             : QStringLiteral("当前程序不支持 EXR，请选择转换后的 png_sequence 目录。");
        return result;
    }

    const QRegularExpression degPattern(QStringLiteral(".*?(\\d+(?:\\.\\d+)?)deg.*"),
                                        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression indexPattern(QStringLiteral("^(\\d+)$"));

    for (const QFileInfo& info : files) {
        const auto match = degPattern.match(info.fileName());
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

    if (!result.frames.isEmpty()) {
        std::sort(result.frames.begin(), result.frames.end(),
                  [](const ImageFrame& a, const ImageFrame& b) { return a.angleDeg < b.angleDeg; });

        for (int i = 1; i < result.frames.size(); ++i) {
            if (std::abs(result.frames[i].angleDeg - result.frames[i - 1].angleDeg) < 1e-6) {
                result.message =
                    QStringLiteral("检测到重复角度: %1°。").arg(result.frames[i].angleDeg, 0, 'f', 6);
                result.ok = false;
                return result;
            }
        }

        if (result.frames.size() < 2) {
            result.message = QStringLiteral("有效图片数量不足，至少需要2张。");
            return result;
        }

        result.suggestedStepDeg = computeSuggestedStepDeg(result.frames);
        result.ok = true;
        result.message = QStringLiteral("序列检查通过: 角度命名序列，有效%1张，忽略%2张，建议步距约%3°。")
                             .arg(result.frames.size())
                             .arg(result.ignoredFileCount)
                             .arg(result.suggestedStepDeg, 0, 'f', 3);
        return result;
    }

    struct IndexedFrame {
        int index = -1;
        QFileInfo info;
    };

    QVector<IndexedFrame> indexedFrames;
    indexedFrames.reserve(files.size());
    result.ignoredFileCount = 0;
    for (const QFileInfo& info : files) {
        const auto match = indexPattern.match(info.completeBaseName());
        if (!match.hasMatch()) {
            ++result.ignoredFileCount;
            continue;
        }

        bool ok = false;
        const int index = match.captured(1).toInt(&ok);
        if (!ok || index < 0) {
            ++result.ignoredFileCount;
            continue;
        }

        indexedFrames.push_back(IndexedFrame{index, info});
    }

    if (indexedFrames.isEmpty()) {
        result.message =
            QStringLiteral("未识别到可用图片命名，请使用 0deg.png 或 0000.png 这类命名。");
        return result;
    }

    std::sort(indexedFrames.begin(), indexedFrames.end(),
              [](const IndexedFrame& a, const IndexedFrame& b) { return a.index < b.index; });

    for (int i = 1; i < indexedFrames.size(); ++i) {
        if (indexedFrames[i].index == indexedFrames[i - 1].index) {
            result.message =
                QStringLiteral("检测到重复序号: %1。").arg(formatFrameIndex(indexedFrames[i].index));
            return result;
        }
    }

    if (indexedFrames.first().index != 0) {
        result.message = QStringLiteral("纯序号序列必须从 0000 开始，当前首帧为 %1。")
                             .arg(formatFrameIndex(indexedFrames.first().index));
        return result;
    }

    for (int i = 1; i < indexedFrames.size(); ++i) {
        const int expected = indexedFrames[i - 1].index + 1;
        if (indexedFrames[i].index != expected) {
            result.message =
                QStringLiteral("纯序号序列序号不连续，缺少 %1。").arg(formatFrameIndex(expected));
            return result;
        }
    }

    if (indexedFrames.size() < 2) {
        result.message = QStringLiteral("有效图片数量不足，至少需要2张。");
        return result;
    }

    result.frames.clear();
    result.frames.reserve(indexedFrames.size());
    result.suggestedStepDeg = 360.0 / static_cast<double>(indexedFrames.size());
    for (const IndexedFrame& indexedFrame : indexedFrames) {
        ImageFrame frame;
        frame.angleDeg = normalizeAngle(indexedFrame.index * result.suggestedStepDeg);
        frame.filePath = indexedFrame.info.absoluteFilePath();
        frame.fileName = indexedFrame.info.fileName();
        result.frames.push_back(frame);
    }

    result.ok = true;
    result.message = QStringLiteral("序列检查通过: 纯序号序列，有效%1张，忽略%2张，按%3°/张均分一圈。")
                         .arg(result.frames.size())
                         .arg(result.ignoredFileCount)
                         .arg(result.suggestedStepDeg, 0, 'f', 3);
    return result;
}
