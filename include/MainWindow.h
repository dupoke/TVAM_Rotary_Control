#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QVector>

#include "BoardAdapter.h"
#include "ConfigManager.h"
#include "ImageSequenceService.h"
#include "LogService.h"
#include "MotionService.h"
#include "ProjectionWindow.h"
#include "ProjectorService.h"
#include "SyncPlaybackService.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    enum class CoarseHomeStage {
        Idle,
        SeekFirstEdge,
        SeekSecondEdge,
        MoveToMidpoint
    };

    void setupUi();
    void initServices();
    void wireSignals();

    void refreshBoardPorts();
    void refreshProjectorPorts();
    bool runImageFolderCheck();
    bool startCoarseHoming();
    void finishCoarseHoming(bool success, const QString& message);
    void updateDiIndicators(quint32 raw);
    void appendUiLog(const QString& message);

    bool parseDoubleInput(QLineEdit* edit, const QString& fieldName, double& outValue);
    bool parseIntInput(QLineEdit* edit, const QString& fieldName, int& outValue);

    QWidget* createDeviceSection();
    QWidget* createMotionSection();
    QWidget* createHomingSection();
    QWidget* createZConfirmSection();
    QWidget* createCameraSection();
    QWidget* createProjectorSyncSection();
    QWidget* createLogSection();

    ConfigManager configManager_;
    AppConfig appConfig_;
    LogService logService_;

    BoardAdapter boardAdapter_;
    MotionService motionService_;
    ProjectorService projectorService_;
    ImageSequenceService imageSequenceService_;
    SyncPlaybackService syncPlaybackService_;
    ProjectionWindow projectionWindow_;

    QVector<ImageFrame> checkedFrames_;

    QPushButton* btnConnectCard_ = nullptr;
    QComboBox* comboBoardPort_ = nullptr;
    QPushButton* btnBoardRefresh_ = nullptr;
    QLabel* lblCardStatus_ = nullptr;
    QLabel* lblComPort_ = nullptr;
    QVector<QLabel*> diIndicators_;

    QLineEdit* editTargetAbsDeg_ = nullptr;
    QLineEdit* editTargetRelDeg_ = nullptr;
    QLabel* lblCurrentDeg_ = nullptr;
    QLabel* lblCurrentPulse_ = nullptr;
    QPushButton* btnCoarseCheck_ = nullptr;
    QPushButton* btnFineHome_ = nullptr;
    QLabel* lblHomeResult_ = nullptr;
    QLabel* lblZeroErrorDeg_ = nullptr;
    QLabel* lblLaserState_ = nullptr;
    QLabel* lblZConfirmTime_ = nullptr;

    QComboBox* comboCameraList_ = nullptr;

    QComboBox* comboProjectorPort_ = nullptr;
    QPushButton* btnProjectorRefresh_ = nullptr;
    QPushButton* btnProjectorOpen_ = nullptr;
    QPushButton* btnProjectorClose_ = nullptr;
    QLabel* lblProjectorPortState_ = nullptr;

    QPushButton* btnProjectorPowerOn_ = nullptr;
    QPushButton* btnProjectorPowerOff_ = nullptr;
    QPushButton* btnLedOn_ = nullptr;
    QPushButton* btnLedOff_ = nullptr;
    QLineEdit* editLedPercent_ = nullptr;
    QPushButton* btnApplyLedPercent_ = nullptr;

    QLineEdit* editImageFolder_ = nullptr;
    QPushButton* btnBrowseImageFolder_ = nullptr;
    QPushButton* btnCheckImageFolder_ = nullptr;
    QLabel* lblImageCheckResult_ = nullptr;
    QLabel* lblCurrentFrame_ = nullptr;

    QSpinBox* spinLoopCount_ = nullptr;
    QCheckBox* chkInfiniteLoop_ = nullptr;
    QLineEdit* editRoundTimeSec_ = nullptr;
    QPushButton* btnSyncStart_ = nullptr;
    QPushButton* btnSyncStop_ = nullptr;
    QLabel* lblSyncState_ = nullptr;
    QLabel* lblSyncProgress_ = nullptr;

    QPlainTextEdit* textLog_ = nullptr;

    QTimer boardPortScanTimer_;
    QTimer coarseHomeTimer_;
    bool coarseHoming_ = false;
    int coarseHomeElapsedMs_ = 0;
    CoarseHomeStage coarseHomeStage_ = CoarseHomeStage::Idle;
    bool coarseHomeInitialDi_ = false;
    qint64 coarseHomeFirstEdgePulse_ = 0;
    qint64 coarseHomeSecondEdgePulse_ = 0;
    qint64 coarseHomeMidPulse_ = 0;
};
