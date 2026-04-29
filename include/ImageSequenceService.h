#pragma once

#include <QString>
#include <QVector>

struct ImageFrame {
    double angleDeg = 0.0;
    QString filePath;
    QString fileName;
};

struct SequenceCheckResult {
    bool ok = false;
    QString message;
    QVector<ImageFrame> frames;
    int ignoredFileCount = 0;
    double suggestedStepDeg = 0.0;
};

class ImageSequenceService final {
public:
    SequenceCheckResult checkFolder(const QString& folderPath) const;
};

