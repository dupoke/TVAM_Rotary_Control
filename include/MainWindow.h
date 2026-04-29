#pragma once

#include <QElapsedTimer>
#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <memory>

#include "BoardAdapter.h"
#include "CameraService.h"
#include "ConfigManager.h"
#include "ImageSequenceService.h"
#include "LogService.h"
#include "MotionService.h"
#include "ProjectionWindow.h"
#include "ProjectorService.h"
#include "SyncPlaybackService.h"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QWidget;
class QCloseEvent;
namespace Ui {
class MainWindow;
}

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    enum class CoarseHomeStage {
        Idle,
        SeekEdge,
        StopSettle,
    };

    enum class FineHomeStage {
        Idle,
        SeekFirstEdge,
        SeekSecondEdge,
        MoveToMidpoint,
    };

    void setupUi();
    void initServices();
    void wireSignals();

    void refreshBoardPorts();
    void refreshProjectorPorts();
    bool runImageFolderCheck();
    bool startCoarseHoming();
    void finishCoarseHoming(bool success, const QString& message);
    bool startFineHoming();
    void finishFineHoming(bool success, const QString& message);
    void handleSafetyTrip(const QString& reason);
    void updateBoardDependentUiState();
    void updateJogButtonStyle();
    void shutdownProjector(bool powerOff);
    void stopSyncWorkflow(bool powerOffProjector);
    void updateSyncElapsedLabel(qint64 elapsedMs = -1);
    bool autoConnectBoard();
    bool autoConnectProjector();
    bool resolveJogSpeed(double& outSpeedPulseMs);
    bool laserTriggeredFromRaw(quint32 raw) const;
    bool laserStableForZConfirm() const;
    void updateLaserState(quint32 raw);
    void updateDiIndicators(quint32 raw);
    void appendUiLog(const QString& message);
    void chooseCameraSaveDirectory();

    QWidget* createBoardMotionPanel();
    QWidget* createProjectorSyncPanel();
    QWidget* createCameraPanel();
    QWidget* createProjectionPreviewPanel();
    QWidget* createLogPanel();

    ConfigManager configManager_;
    AppConfig appConfig_;
    LogService logService_;

    BoardAdapter boardAdapter_;
    CameraService cameraService_;
    MotionService motionService_;
    ProjectorService projectorService_;
    ImageSequenceService imageSequenceService_;
    SyncPlaybackService syncPlaybackService_;
    ProjectionWindow projectionWindow_;

    QVector<ImageFrame> checkedFrames_;
    QWidget* motionSectionWidget_ = nullptr;

    QPushButton* btnConnectCard_ = nullptr;
    QComboBox* comboBoardPort_ = nullptr;
    QPushButton* btnBoardRefresh_ = nullptr;
    QLabel* lblCardStatus_ = nullptr;
    QLabel* lblComPort_ = nullptr;
    QVector<QLabel*> diIndicators_;

    QDoubleSpinBox* editTargetAbsDeg_ = nullptr;
    QDoubleSpinBox* editTargetRelDeg_ = nullptr;
    QPushButton* btnJogPlusMotion_ = nullptr;
    QPushButton* btnJogMinusMotion_ = nullptr;
    QPushButton* btnStopMotion_ = nullptr;
    QPushButton* btnEStopMotion_ = nullptr;
    QPushButton* btnMoveAbsMotion_ = nullptr;
    QPushButton* btnMoveRelMotion_ = nullptr;
    QLabel* lblCurrentDeg_ = nullptr;
    QLabel* lblCurrentPulse_ = nullptr;
    QLabel* lblMotionConfig_ = nullptr;
    QDoubleSpinBox* editJogSpeedPulseMs_ = nullptr;
    QPushButton* btnCoarseCheck_ = nullptr;
    QPushButton* btnFineHome_ = nullptr;
    QPushButton* btnConfirmZReady_ = nullptr;
    QLabel* lblHomeResult_ = nullptr;
    QLabel* lblZeroErrorDeg_ = nullptr;
    QLabel* lblLaserState_ = nullptr;
    QLabel* lblZConfirmTime_ = nullptr;

    QComboBox* comboCameraList_ = nullptr;
    QPushButton* btnCameraOpen_ = nullptr;
    QPushButton* btnCameraRefresh_ = nullptr;
    QPushButton* btnSnap_ = nullptr;
    QPushButton* btnRec_ = nullptr;
    QPushButton* btnCameraModeToggle_ = nullptr;
    QLineEdit* editCameraSaveDir_ = nullptr;
    QLabel* lblCameraRecordTime_ = nullptr;
    QLabel* lblCameraState_ = nullptr;
    QDoubleSpinBox* spinCameraBrightness_ = nullptr;
    QDoubleSpinBox* spinCameraContrast_ = nullptr;
    QDoubleSpinBox* spinCameraExposure_ = nullptr;
    QDoubleSpinBox* spinCameraSaturation_ = nullptr;

    QComboBox* comboProjectorPort_ = nullptr;
    QPushButton* btnProjectorRefresh_ = nullptr;
    QPushButton* btnProjectorOpen_ = nullptr;
    QPushButton* btnProjectorClose_ = nullptr;
    QLabel* lblProjectorPortState_ = nullptr;

    QPushButton* btnProjectorPowerOn_ = nullptr;
    QPushButton* btnProjectorPowerOff_ = nullptr;
    QPushButton* btnLedOn_ = nullptr;
    QPushButton* btnLedOff_ = nullptr;
    QPushButton* btnAlignSiemens_ = nullptr;
    QSpinBox* editLedPercent_ = nullptr;
    QPushButton* btnApplyLedPercent_ = nullptr;

    QLineEdit* editImageFolder_ = nullptr;
    QPushButton* btnBrowseImageFolder_ = nullptr;
    QPushButton* btnCheckImageFolder_ = nullptr;
    QLabel* lblImageCheckResult_ = nullptr;
    QLabel* lblCurrentFrame_ = nullptr;
    QLabel* lblProjectionPreview_ = nullptr;

    QSpinBox* spinLoopCount_ = nullptr;
    QCheckBox* chkInfiniteLoop_ = nullptr;
    QDoubleSpinBox* editRoundTimeSec_ = nullptr;
    QPushButton* btnSyncStart_ = nullptr;
    QPushButton* btnSyncStop_ = nullptr;
    QLabel* lblSyncState_ = nullptr;
    QLabel* lblSyncProgress_ = nullptr;
    QLabel* lblSyncElapsed_ = nullptr;

    QPlainTextEdit* textLog_ = nullptr;

    QTimer boardPortScanTimer_;
    QTimer coarseHomeTimer_;
    QTimer fineHomeTimer_;
    QTimer syncElapsedUiTimer_;
    bool coarseHoming_ = false;
    bool fineHoming_ = false;
    bool safetyTripped_ = false;
    QString lastBoardPort_;
    QString lastProjectorPort_;
    quint32 lastDiRaw_ = 0;
    int jogUiDirection_ = 0;
    double coarseHomeJogSpeedPulseMs_ = 0.0;
    int coarseHomeElapsedMs_ = 0;
    int coarseHomeConfirmRemainMs_ = 0;
    qint64 coarseHomeStartPulse_ = 0;
    CoarseHomeStage coarseHomeStage_ = CoarseHomeStage::Idle;
    bool coarseHomeInitialDi_ = false;
    qint64 coarseHomeFirstEdgePulse_ = 0;
    qint64 coarseHomeConfirmPulse_ = 0;
    double fineHomeJogSpeedPulseMs_ = 0.0;
    int fineHomeElapsedMs_ = 0;
    qint64 fineHomeStartPulse_ = 0;
    qint64 fineHomeFirstEdgePulse_ = 0;
    qint64 fineHomeSecondEdgePulse_ = 0;
    qint64 fineHomeMidPulse_ = 0;
    FineHomeStage fineHomeStage_ = FineHomeStage::Idle;
    bool fineHomeLastLaserState_ = false;
    bool laserTriggered_ = false;
    QElapsedTimer laserStableTimer_;
    bool cameraMonochrome_ = false;
    bool cameraAutoOpenEvaluated_ = false;
    QElapsedTimer syncElapsedTimer_;
    qint64 syncElapsedMs_ = 0;
    std::unique_ptr<Ui::MainWindow> ui_;
};
