#ifndef FLOATINGWORKOUT_H
#define FLOATINGWORKOUT_H

#include <QDialog>
#include <QPropertyAnimation>

#include <dataworkout.h>
#include <faderframe.h>
#include <faderlabel.h>
#include <hub.h>
#include <interval.h>
#include <powercurve.h>
#include <sensor.h>
#include <soundplayer.h>
#include <workout.h>
#include <workoutplot.h>
#include <workoutplotzoomer.h>

#include "oxygen_controller.h"
#include "myconstants.h"

namespace Ui {
class FloatingWorkout;
}

class FloatingWorkout : public QDialog
{
    Q_OBJECT

public:
    explicit FloatingWorkout(QVector<Hub*> vechub, QVector<int> vecStickIdUsed, QList<Sensor> lstSensor, Workout workout, QWidget *parent = nullptr);
    ~FloatingWorkout();

    void reject();

    void showHeartRateDisplayWidget(int display);
    void showPowerDisplayWidget(int display);
    void showCadenceDisplayWidget(int display);


    void setMessagePlot();

    ///timers
    void slotGetSensorListFinished(QList<Sensor> lstSensor);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
signals:
    ///Clock
    void startClock();
    void pauseClock();
    void resumeClock();
    void finishClock();
    void sendUserInfo(int userID, double cda, double weight, int nbUser);
    void sendPowerData(int userID, int power);
    void sendSlopeData(int userID, double slope);
    void startClockSpeed();


    //--Send to Hub
    void sendSoloData(PowerCurve curve, int wheelCircMM, QList<Sensor> lstSensor, bool usePmForCadence, bool usePmForSpeed);

    void stopDecodingMsgHub();
    void sendOxygenCommand(int antID, Oxygen_Controller::COMMAND);

    // Commands to FE-C
    void setLoad(int antID, double load);
    void setSlope(int antID, double slope);

    void increaseDifficulty();
    void decreaseDifficulty();

    void permissionGrantedControl(int antID, int hubID);



    void ftp_lthr_changed();
    void fitFileReady(QString filename, QString nameOnly, QString description);

    void playPlayer();
    void pausePlayer();


    void insideConfig(bool inside);


public slots:
    // control list master of QThreads hub
    void addToControlList(int antID, int fromHubNumber);

    ///Connected to Clock
    void update1sec(double totalTimeElapsed_sec);
    void updateRealTimeGraph(double totalTimeElapsed_msec);
    void updatePausedTime(double totalTimePaused_msec);

    void lapButtonPressed();


    void batteryStatusReceived(QString sensorType, int level, int antID);


    void HrDataReceived(int userID, int value);
    void CadenceDataReceived(int userID, int value);
    void PowerDataReceived(int userID, int value);
    void pedalMetricReceived(int userID, double,double,double,double,double);

    void start_or_pause_workout();

    void ignoreClickPlot();

    void activateSoundBool();
    void activateSoundPowerTooLow();
    void activateSoundPowerTooHigh();
    void activateSoundCadenceTooLow();
    void activateSoundCadenceTooHigh();

    void targetDiffTypeChanged(int diffLevel);


private slots:

    void closeWindow();
    void minimizeWindow();
    void expandWindow();
    void toggleTransparent();

    void insertInterval();
    void moveToInterval(int nb, double secWorkout, double startIntervalSec, bool showConfirmation);
    void adjustWorkoutDifficulty(int percentage);

    void changeWidget();
    void hrCadAnimationFinished();

    void fakeData();


    void on_button_close_clicked();

    void setWidgetBackgroundColor(QColor *color);

    void alphaAnimationFinished();
    void backgroundColorChanged(QVariant value);

private:

    void checkFitFileCreated();
    void closeFitFiles(double timeElapsed_sec);
    void closeAndDeleteFitFile();
    void changeIntervalsDataWorkout(double, double, int, bool, bool);
    void initDataWorkout();
    void connectDataWorkout();

    void connectHubs();
    void sendUserInfoToClock();

    void showTestResult();



    void sendTargetsPower(double percentageTarget, int range);
    void sendTargetsCadence(int target, int range);
    void sendTargetsHr(double percentageTarget, int range);

    void sendLoads(double percentageFTP);
    void sendSlopes(double slope);


    void showFullScreenWin();
    void showNormalWin();

    void animateAchievement();
    void checkMAPTestOver();


    bool eventFilter(QObject *watched, QEvent *event);

    void initUI();
    double calculateNewTargetPower();
    int calculateNewTargetCadence();
    double calculateNewTargetHr();
    void startWorkout();
    void workoutOver();
    void moveToNextInterval();

    void targetPowerChanged_f(double percentageTarget, int range);
    void targetCadenceChanged_f(int target, int range);
    void targetHrChanged_f(double percentageTarget, int range);
    void setWidgetsStopped(bool b);


    void sendLastSecondData(int seconds);
    void adjustTargets(Interval interval);

    void sureYouWantToQuit();


    void toggleWidget(QWidget *widget, QPropertyAnimation *animation, QGraphicsOpacityEffect *effect, bool isShow);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void loadPosFromSettings();
    void savePosSettings();

private:
    Ui::FloatingWorkout *ui;
    bool isUsingSlopeMode;
    QTimer *timerAlertCalibrateCt;

    ///Clock thread
    QThread *thread;

    /// Pairing window
    QWidget *widgetLoading;
    FaderLabel *labelPairHr;
    FaderLabel *labelSpeedCadence;
    FaderLabel *labelCadence;
    FaderLabel *labelSpeed;
    FaderLabel *labelFEC;
    FaderLabel *labelOxygen;
    FaderLabel *labelPower;
    FaderLabel *labelCtPower;

    /// Calibrate window + battery status
    QWidget *widgetBattery;
    FaderLabel *labelBattery;
    FaderLabel *labelBatteryStatus;

    bool isAskingUserQuestion;
    bool isCalibrating;


    QString msgPairingDone;
    bool hrPairingDone;
    bool scPairingDone;
    bool cadencePairingDone;
    bool speedPairingDone;
    bool powerPairingDone;
    bool fecPairingDone;
    bool oxygenPairingDone;


    /// Identify sensors (solo)
    Sensor sensorHr;
    Sensor sensorSpeedCadence;
    Sensor sensorCadence;
    Sensor sensorSpeed;
    Sensor sensorPower;
    Sensor sensorFEC;
    Sensor sensorOxygen;


    bool usingHr;
    bool usingSpeedCadence;
    bool usingCadence;
    bool usingSpeed;
    bool usingPower;
    bool usingPowerSensorForCadence;
    bool usingPowerSensorForSpeed;
    bool usingFEC;
    bool usingOxygen;

    int idFecMainUser;


    void checkPairingCompleted();
    /////////////////////////


    QString bakStylesheet;
    bool isTransparent;
    bool wasFullscreenOnOpen;
    bool isFullScreenFlag;
    QSize lastNormalSize;


    QVector<Hub*> vecHub;
    QVector<int> vecStickIdUsed;
    QHash<int,int> hashControlList; //addId, hubId
    //    int nbTotalFecTrainer;


    Workout workout;
    bool isTestWorkout;
    QVector<UserStudio> vecUserStudio;
    //DataWorkout *dataWorkout; //Pointer because is a QObject (needs Signal) - deletion is automatic
    DataWorkout *arrDataWorkout[constants::nbMaxUserStudio];

    QTimer *timer_sec;

    Account *account;


    // For Rolling Averaging Power
    //    QQueue<double> queuePower;
    //    int lastSecondPower;
    //    int nbPointsPower;

    QQueue<double> arrQueuePower[constants::nbMaxUserStudio];
    int arrLastSecondPower[constants::nbMaxUserStudio];
    int arrNbPointPower[constants::nbMaxUserStudio];

    //to calculate mean every second ,[0] = Rider#1
    QVector<int> nbPointHr1sec;
    QVector<double> averageHr1sec;
    QVector<int> nbPointCadence1sec;
    QVector<double> averageCadence1sec;
    QVector<int> nbPointSpeed1sec;
    QVector<double> averageSpeed1sec;
    QVector<int> nbPointPower1sec;
    QVector<double> averagePower1sec;

    //not doing average for now
    QVector<double> avgRightPedal1sec;
    QVector<double> avgLeftTorqueEff;
    QVector<double> avgRightTorqueEff;
    QVector<double> avgLeftPedalSmooth;
    QVector<double> avgRightPedalSmooth;
    QVector<double> avgCombinedPedalSmooth;
    QVector<double> avgSaturatedHemoglobinPercent1sec;
    QVector<double> avgTotalHemoglobinConc1sec;




    double lastIntervalEndTime_sec;

    //to determine pausedTime
    int lastIntervalTotalTimePausedWorkout_msec;
    int totalTimePausedWorkout_msec;

    int lastIntervalEndTime_msec;
    double timeElapsed_sec;
    //    double skippedTime_sec;

    int nbUpdate1Sec;
    QTime timeWorkoutRemaining;
    QTime timeElapsedTotal;
    QTime timeInterval;



    bool isWorkoutStarted;
    bool isWorkoutOver;
    bool isWorkoutPaused;


    /// disable screen saver
#ifdef Q_OS_MAC
    MacUtils macUtil;
#endif

    bool bignoreClickPlot;
    QTimer *timerIgnoreClickPlot;

    ///------ Sounds ----------------------
    QTimer *timerCheckToActivateSound;
    bool soundsActive;

    int durationReactivateSameSoundMsec; //to edit if 10sec is too long
    QTimer *timerCheckReactivateSoundPowerTooLow;
    QTimer *timerCheckReactivateSoundPowerTooHigh;
    QTimer *timerCheckReactivateSoundCadenceTooLow;
    QTimer *timerCheckReactivateSoundCadenceTooHigh;
    bool soundPowerTooLowActive;
    bool soundPowerTooHighActive;
    bool soundCadenceTooLowActive;
    bool soundCadenceTooHighActive;
    ///---------------------------------------

    int currMAPInterval;
    int totalSecOffTargetInInterval;
    int totalConsecutiveOffTarget;

    int currentTargetPower;
    int currentTargetPowerRange;
    int currentTargetCadence;
    int currentTargetCadenceRange;

    Interval currentIntervalObj;
    int currentInterval;
    bool changeIntervalDisplayNextSecond;
    bool ignoreCondition;

    int currentWorkoutDifficultyPercentage; //0 = Normal


    WorkoutPlot *mainPlot;
    friend class DialogConfig;
    SoundPlayer *soundPlayer;

    QTimer *widgetChangeTimer;
    QPropertyAnimation *plotAnimation;
    QPropertyAnimation *hrCadAnimation;

    QGraphicsOpacityEffect * hrOpacityEffect;
    QGraphicsOpacityEffect * cadOpacityEffect;
    QGraphicsOpacityEffect * plotOpacityEffect;
    QGraphicsOpacityEffect * zoomPlotOpacityEffect;

    QTimer *createFakeDataTimer;
    int originFakePower = 0;
    qint64 lastFakeChangeTime;

    bool isMouseDown = false;
    QPoint m_startPoint;
    QPoint m_windowPoint;

    int diffLevel;
    QVariantAnimation *colorAlphaAnimation;
    QParallelAnimationGroup *colorRGBAnimation;
    int currentAlpha;
    int currentR;
    int currentG;
    int currentB;
    const int inrangeWhite = 200;
};

#endif // FLOATINGWORKOUT_H
