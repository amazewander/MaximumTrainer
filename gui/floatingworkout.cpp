#include "floatingworkout.h"
#include "ui_floatingworkout.h"
#include "datacadence.h"
#include "datapower.h"
#include "dataheartrate.h"
#include "dataspeed.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QDesktopWidget>
#include <clock.h>
#include <util.h>
#include <QDateTime>


FloatingWorkout::~FloatingWorkout() {

    qDebug() << "Floating Workout Dialog destructor!----------";

    emit stopDecodingMsgHub();


    delete ui;

#ifdef Q_OS_MAC
    macUtil.releaseScreensaverLock();
#endif
#ifdef Q_OS_WIN32
    SetThreadExecutionState(ES_CONTINUOUS);
#endif

    // stop and delete Clock thread
    emit finishClock();


    // Wait for clock thread to stop
    thread->quit();
    thread->wait();

    DataCadence::instance().clearData();
    DataPower::instance().clearData();
    DataHeartRate::instance().clearData();
    DataSpeed::instance().clearData();

    qDebug() << "----Destructor over Floating Workout Dialog";
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FloatingWorkout::FloatingWorkout(QVector<Hub*> paraVecHub, QVector<int> paraVecStickIdUsed, QList<Sensor> lstSensor, Workout paraWorkout,
                             QWidget *parent) : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint), ui(new Ui::FloatingWorkout) {

    ui->setupUi(this);
    //setWindowOpacity(0.5);
    int dialogWidth = QApplication::desktop()->height()/3;
    setFixedSize(dialogWidth, dialogWidth);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_DeleteOnClose);

    int plotWidth = dialogWidth - 22;
    ui->widget_workoutPlot->setFixedWidth(plotWidth);
    ui->wid_2_workoutPlot_PowerZoom->setFixedWidth(plotWidth);

    //ui->scrollArea->setb


    this->setFocusPolicy(Qt::ClickFocus);
    // Disable ScreenSaver
#ifdef Q_OS_MAC
    macUtil.disableScreensaver();
#endif
#ifdef Q_OS_WIN32
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
#endif


    this->vecHub = paraVecHub;
    this->vecStickIdUsed = paraVecStickIdUsed;
    this->account = qApp->property("Account").value<Account*>();
    this->soundPlayer =  qApp->property("SoundPlayer").value<SoundPlayer*>();
    this->workout = paraWorkout;
    soundPlayer->setVolume(account->sound_player_vol);
    msgPairingDone = tr("Done!");


    //Init
    usingHr = false;
    usingSpeedCadence = false;
    usingCadence = false;
    usingSpeed = false;
    usingPower = false;
    usingFEC = false;
    usingOxygen = false;

    hrPairingDone = false;
    scPairingDone = false;
    cadencePairingDone = false;
    speedPairingDone = false;
    powerPairingDone = false;
    fecPairingDone = false;
    oxygenPairingDone = false;

    idFecMainUser = -1;

    isUsingSlopeMode = false;
    timerAlertCalibrateCt = new QTimer(this);

    //data points
    nbPointHr1sec = QVector<int>(constants::nbMaxUserStudio, 0);
    averageHr1sec = QVector<double>(constants::nbMaxUserStudio, -1);

    nbPointCadence1sec = QVector<int>(constants::nbMaxUserStudio, 0);
    averageCadence1sec = QVector<double>(constants::nbMaxUserStudio, -1);

    nbPointSpeed1sec = QVector<int>(constants::nbMaxUserStudio, 0);
    averageSpeed1sec = QVector<double>(constants::nbMaxUserStudio, -1);

    nbPointPower1sec = QVector<int>(constants::nbMaxUserStudio, 0);
    averagePower1sec = QVector<double>(constants::nbMaxUserStudio, -1);

    avgRightPedal1sec = QVector<double>(constants::nbMaxUserStudio, -1);
    avgLeftTorqueEff = QVector<double>(constants::nbMaxUserStudio, -1);
    avgRightTorqueEff = QVector<double>(constants::nbMaxUserStudio, -1);
    avgLeftPedalSmooth = QVector<double>(constants::nbMaxUserStudio, -1);
    avgRightPedalSmooth = QVector<double>(constants::nbMaxUserStudio, -1);
    avgCombinedPedalSmooth = QVector<double>(constants::nbMaxUserStudio, -1);
    avgSaturatedHemoglobinPercent1sec = QVector<double>(constants::nbMaxUserStudio, -1);
    avgTotalHemoglobinConc1sec = QVector<double>(constants::nbMaxUserStudio, -1);



    //// ------------- Clock thread -------------------------
    thread = new QThread(this);
    Clock *clock1 = new Clock("clock1");
    clock1->moveToThread(thread);

    ///This --> Clock
    connect(this, SIGNAL(startClock()), clock1, SLOT(startClock()) );
    connect(this, SIGNAL(pauseClock()), clock1, SLOT(pauseClock()) );
    connect(this, SIGNAL(resumeClock()), clock1, SLOT(resumeClock()) );
    connect(this, SIGNAL(finishClock()), clock1, SLOT(finishClock()) );

    connect(this, SIGNAL(sendUserInfo(int, double,double,int)), clock1, SLOT(receiveUserInfo(int, double,double,int)) );
    connect(this, SIGNAL(sendPowerData(int, int)), clock1, SLOT(receivePowerData(int, int)) );
    //    connect(this, SIGNAL(sendSlopeData(int,double)), clock1, SLOT(receiveSlopeData(int, double)) );
    connect(this, SIGNAL(startClockSpeed()), clock1, SLOT(startClockSpeed()) );

    ///Clock --> This
    connect(clock1, SIGNAL(oneCyclePassed(double)), this, SLOT(updateRealTimeGraph(double)) );
    connect(clock1, SIGNAL(oneSecPassed(double)), this, SLOT(update1sec(double)) );
    connect(clock1, SIGNAL(updateTimePaused(double)), this, SLOT(updatePausedTime(double)) );

    connect(clock1, SIGNAL(finished()), thread, SLOT(quit()));
    connect(clock1, SIGNAL(finished()), clock1, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
    ///----------------------------------------------------
    timeElapsed_sec = 0;
    lastIntervalEndTime_msec = 0;

    lastIntervalTotalTimePausedWorkout_msec = 0;
    totalTimePausedWorkout_msec = 0;
    lastIntervalEndTime_sec = 0;


    sendUserInfoToClock();
    emit startClockSpeed();

    // Connect all Hubs (one per ANT stick)
    connectHubs();


    // Ignore click on workout plot while widget is loading
    timerIgnoreClickPlot = new QTimer(this);
    bignoreClickPlot = true;
    timerIgnoreClickPlot->start(1000);
    connect(timerIgnoreClickPlot, SIGNAL(timeout()), this, SLOT(ignoreClickPlot()) );


    /// Sounds timer
    timerCheckToActivateSound = new QTimer(this);
    timerCheckToActivateSound->setInterval(4000);
    connect(timerCheckToActivateSound, SIGNAL(timeout()), this, SLOT(activateSoundBool()) );
    soundsActive = false;

    durationReactivateSameSoundMsec = 10000; //to edit if 10sec is too long
    timerCheckReactivateSoundPowerTooLow =  new QTimer(this);
    timerCheckReactivateSoundPowerTooHigh =  new QTimer(this);
    timerCheckReactivateSoundCadenceTooLow =  new QTimer(this);
    timerCheckReactivateSoundCadenceTooHigh =  new QTimer(this);
    timerCheckReactivateSoundPowerTooLow->setInterval(durationReactivateSameSoundMsec);
    timerCheckReactivateSoundPowerTooHigh->setInterval(durationReactivateSameSoundMsec);
    timerCheckReactivateSoundCadenceTooLow->setInterval(durationReactivateSameSoundMsec);
    timerCheckReactivateSoundCadenceTooHigh->setInterval(durationReactivateSameSoundMsec);
    connect(timerCheckReactivateSoundPowerTooLow, SIGNAL(timeout()), this, SLOT(activateSoundPowerTooLow()) );
    connect(timerCheckReactivateSoundPowerTooHigh, SIGNAL(timeout()), this, SLOT(activateSoundPowerTooHigh()) );
    connect(timerCheckReactivateSoundCadenceTooLow, SIGNAL(timeout()), this, SLOT(activateSoundCadenceTooLow()) );
    connect(timerCheckReactivateSoundCadenceTooHigh, SIGNAL(timeout()), this, SLOT(activateSoundCadenceTooHigh()) );
    soundPowerTooLowActive = true;
    soundPowerTooHighActive = true;
    soundCadenceTooLowActive = true;
    soundCadenceTooHighActive = true;



    ///-------------------------- Widget Sensor Loading  ----------------------
    widgetLoading = new QWidget(this);
    widgetLoading->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    widgetLoading->setFocusPolicy(Qt::NoFocus);
    QVBoxLayout *vLayout = new QVBoxLayout(widgetLoading);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);

    QSpacerItem *spacer = new QSpacerItem(200, 200, QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont fontLabel;
    fontLabel.setPointSize(10);
    labelPairHr = new FaderLabel(widgetLoading);
    labelPairHr->setFont(fontLabel);
    labelPairHr->setMinimumHeight(20);
    labelPairHr->setMaximumHeight(20);
    labelPairHr->setMaximumWidth(600);
    labelPairHr->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelPairHr->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelPairHr->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelPairHr->setStyleSheet("background-color : rgba(1,1,1,220); color : white;");

    labelSpeedCadence = new FaderLabel(widgetLoading);
    labelSpeedCadence->setFont(fontLabel);
    labelSpeedCadence->setMinimumHeight(20);
    labelSpeedCadence->setMaximumHeight(20);
    labelSpeedCadence->setMaximumWidth(600);
    labelSpeedCadence->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelSpeedCadence->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelSpeedCadence->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelSpeedCadence->setStyleSheet("background-color : rgba(1,1,1,220); color : white;");

    labelCadence = new FaderLabel(widgetLoading);
    labelCadence->setFont(fontLabel);
    labelCadence->setMinimumHeight(20);
    labelCadence->setMaximumHeight(20);
    labelCadence->setMaximumWidth(600);
    labelCadence->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelCadence->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelCadence->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelCadence->setStyleSheet("background-color : rgba(1,1,1,220); color : white;");

    labelSpeed = new FaderLabel(widgetLoading);
    labelSpeed->setFont(fontLabel);
    labelSpeed->setMinimumHeight(20);
    labelSpeed->setMaximumHeight(20);
    labelSpeed->setMaximumWidth(600);
    labelSpeed->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelSpeed->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelSpeed->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelSpeed->setStyleSheet("background-color : rgba(1,1,1,220); color : white;");

    labelFEC = new FaderLabel(widgetLoading);
    labelFEC->setFont(fontLabel);
    labelFEC->setMinimumHeight(20);
    labelFEC->setMaximumHeight(20);
    labelFEC->setMaximumWidth(600);
    labelFEC->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelFEC->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelFEC->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelFEC->setStyleSheet("background-color : rgba(1,1,1,220); color : white;");

    labelOxygen = new FaderLabel(widgetLoading);
    labelOxygen->setFont(fontLabel);
    labelOxygen->setMinimumHeight(20);
    labelOxygen->setMaximumHeight(20);
    labelOxygen->setMaximumWidth(600);
    labelOxygen->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelOxygen->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelOxygen->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelOxygen->setStyleSheet("background-color : rgba(1,1,1,220); color : white;");

    labelPower = new FaderLabel(widgetLoading);
    labelPower->setFont(fontLabel);
    labelPower->setMinimumHeight(20);
    labelPower->setMaximumHeight(20);
    labelPower->setMaximumWidth(600);
    labelPower->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelPower->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelPower->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelPower->setStyleSheet("background-color : rgba(1,1,1,220); color : white;");

    labelCtPower = new FaderLabel(widgetLoading);
    labelCtPower->setFont(fontLabel);
    labelCtPower->setMinimumHeight(20);
    labelCtPower->setMaximumHeight(20);
    labelCtPower->setMaximumWidth(600);
    labelCtPower->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    labelCtPower->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    labelCtPower->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    labelCtPower->setStyleSheet("background-color : rgba(1,1,1,220); color : green;");

    labelPairHr->setVisible(false);
    labelSpeedCadence->setVisible(false);
    labelCadence->setVisible(false);
    labelSpeed->setVisible(false);
    labelFEC->setVisible(false);
    labelOxygen->setVisible(false);
    labelPower->setVisible(false);
    labelCtPower->setVisible(false);

    vLayout->addSpacerItem(spacer);
    vLayout->addWidget(labelPairHr, Qt::AlignBottom);
    vLayout->addWidget(labelSpeedCadence, Qt::AlignBottom);
    vLayout->addWidget(labelCadence, Qt::AlignBottom);
    vLayout->addWidget(labelSpeed, Qt::AlignBottom);
    vLayout->addWidget(labelFEC, Qt::AlignBottom);
    vLayout->addWidget(labelOxygen, Qt::AlignBottom);
    vLayout->addWidget(labelPower, Qt::AlignBottom);
    vLayout->addWidget(labelCtPower, Qt::AlignBottom);


    QGridLayout *glayout = static_cast<QGridLayout*>( this->layout()  );
    glayout->addWidget(widgetLoading, 0, 0, 0, 0);


    widgetLoading->setAttribute(Qt::WA_TransparentForMouseEvents,true);
    widgetLoading->setWindowFlags(Qt::WindowStaysOnTopHint);
    ///----------------------------- End Calibration widgets ------------------------


    if (vecStickIdUsed.size() > 0) {
        labelPairHr->setText(tr(" Checking Session..."));
        labelPairHr->setVisible(true);
        labelPairHr->fadeIn(500);
    }
    else {
        labelPairHr->setStyleSheet("background-color : rgba(1,1,1,220); color : red;");
        labelPairHr->setText(tr(" No ANT+ USB Stick was detected, Make sure it is connected and that no other program is using it."));
        labelPairHr->setVisible(true);
        labelPairHr->fadeInAndFadeOutAfterPause(500,2000,7000);
    }


    isTransparent = false;
//    Qt::WindowFlags flags;
//    if (account->force_workout_window_on_top)
//        flags = flags | Qt::WindowStaysOnTopHint;

//#ifdef Q_OS_MAC
//    flags = flags | Qt::Window;
//#else
//    flags = flags | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint;
//#endif
//    this->setWindowFlags(flags);
    installEventFilter(this); //For Hotkeys


    connect(this, SIGNAL(increaseDifficulty()), ui->widget_workoutPlot, SLOT(increaseDifficulty()) );
    connect(this, SIGNAL(decreaseDifficulty()), ui->widget_workoutPlot, SLOT(decreaseDifficulty()) );
    connect(ui->widget_workoutPlot, SIGNAL(workoutDifficultyChanged(int)), this, SLOT(adjustWorkoutDifficulty(int)) );
    connect(ui->widget_workoutPlot, SIGNAL(intervalClicked(int,double,double,bool)), this, SLOT(moveToInterval(int,double,double,bool)) );



    // Initialise DataWorkout
    initDataWorkout();
    // Connect DataWorkout
    connectDataWorkout();




    //Minimalist
    ui->wid_1_minimalistHr->setTypeWidget(MinimalistWidget::HEART_RATE);
    ui->wid_2_minimalistPower->setTypeWidget(MinimalistWidget::POWER);
    ui->wid_3_minimalistCadence->setTypeWidget(MinimalistWidget::CADENCE);

    //Set User Data in all widgets
    ui->widget_workoutPlot->setUserData(account->getFTP(), account->getLTHR());
    ui->wid_2_workoutPlot_PowerZoom->setUserData(account->getFTP(), account->getLTHR());
    ui->wid_1_minimalistHr->setUserData(account->getFTP(), account->getLTHR());
    ui->wid_2_minimalistPower->setUserData(account->getFTP(), account->getLTHR());
    ui->wid_3_minimalistCadence->setUserData(account->getFTP(), account->getLTHR());
    // Set Workout Data to widgets
    ui->widget_workoutPlot->setWorkoutData(workout, true);
    ui->wid_2_workoutPlot_PowerZoom->setWorkoutData(workout, WorkoutPlotZoomer::POWER, true);


    mainPlot = ui->widget_workoutPlot;

    qDebug() << "GOT HERE WORKOTUDI1";


    currentWorkoutDifficultyPercentage = 0;

    isAskingUserQuestion = false;
    isCalibrating = false;
    isWorkoutStarted = false;
    isWorkoutPaused = true;
    isWorkoutOver = false;
    changeIntervalDisplayNextSecond = false;
    ignoreCondition = false;
    if (workout.getWorkoutNameEnum() == Workout::OPEN_RIDE) {
        currentInterval = -1;
        isUsingSlopeMode = true;
    }
    else {
        currentInterval = 0;
        currentIntervalObj = workout.getInterval(currentInterval);
    }



    //    lastSecondPower = 0;
    //    nbPointsPower = 0;
    //New
    for (int i=0; i<constants::nbMaxUserStudio; i++) {
        arrLastSecondPower[i] = 0;
        arrNbPointPower[i] = 0;
    }

    timeElapsedTotal = QTime(0,0,0,0);
    nbUpdate1Sec = 0;

    idFecMainUser = -1;
    currentTargetPower = -1;
    currentTargetPowerRange = -1;
    currentTargetCadence = -1;
    currentTargetCadenceRange = -1;
    currMAPInterval = 1;
    totalSecOffTargetInInterval = 0;
    totalConsecutiveOffTarget = 0;



    //Disable button in Test mode
    if (workout.getWorkoutNameEnum() == Workout::FTP_TEST || workout.getWorkoutNameEnum() == Workout::FTP8min_TEST ||
            workout.getWorkoutNameEnum() == Workout::CP5min_TEST || workout.getWorkoutNameEnum() == Workout::CP20min_TEST ||
            workout.getWorkoutNameEnum() == Workout::MAP_TEST ) {
        isTestWorkout = true;
    }
    else {
        isTestWorkout = false;
    }


    initUI();
    setWidgetsStopped(true);

    this->setFocus();

    // Remove loading Cursor
    QApplication::restoreOverrideCursor();

    //TODO
    this->loadPosFromSettings();

    ui->wid_1_minimalistHr->setVisible(false);
    //ui->wid_2_workoutPlot_PowerZoom->setVisible(false);
    //ui->widget_workoutPlot->setVisible(false);

    ui->button_close->hide();

    hrOpacityEffect = new QGraphicsOpacityEffect(ui->wid_1_minimalistHr);
    hrOpacityEffect->setObjectName("hrOpacityEffect");
    hrOpacityEffect->setOpacity(1);
    ui->wid_1_minimalistHr->setGraphicsEffect(hrOpacityEffect);

    cadOpacityEffect = new QGraphicsOpacityEffect(ui->wid_3_minimalistCadence);
    cadOpacityEffect->setObjectName("cadOpacityEffect");
    ui->wid_3_minimalistCadence->setGraphicsEffect(cadOpacityEffect);

    plotOpacityEffect = new QGraphicsOpacityEffect(ui->widget_workoutPlot);
    plotOpacityEffect->setObjectName("plotOpacityEffect");
    ui->widget_workoutPlot->setGraphicsEffect(plotOpacityEffect);

    zoomPlotOpacityEffect = new QGraphicsOpacityEffect(ui->wid_2_workoutPlot_PowerZoom);
    zoomPlotOpacityEffect->setObjectName("zoomPlotOpacityEffect");
    ui->wid_2_workoutPlot_PowerZoom->setGraphicsEffect(zoomPlotOpacityEffect);

    widgetChangeTimer = new QTimer(this);
    connect(widgetChangeTimer, SIGNAL(timeout()), this, SLOT(changeWidget()));
    plotAnimation = new QPropertyAnimation(ui->scrollArea->horizontalScrollBar(), "value");
    //plotAnimation->setPropertyName("value");
    plotAnimation->setDuration(300);
    hrCadAnimation = new QPropertyAnimation();
    hrCadAnimation->setPropertyName("opacity");
    hrCadAnimation->setDuration(300);
    //connect(plotAnimation, SIGNAL(finished()), this, SLOT(plotAnimationFinished()));
    connect(hrCadAnimation, SIGNAL(finished()), this, SLOT(hrCadAnimationFinished()));


    connect(ui->wid_2_workoutPlot_PowerZoom, SIGNAL(diffTypeChanged(int)), this, SLOT(targetDiffTypeChanged(int)));

    this->colorRGBAnimation = new QParallelAnimationGroup();
    QVariantAnimation *redAnimation = new QVariantAnimation();
    redAnimation->setDuration(1000);
    redAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    QVariantAnimation *greenAnimation = new QVariantAnimation();
    greenAnimation->setDuration(1000);
    greenAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    QVariantAnimation *blueAnimation = new QVariantAnimation();
    blueAnimation->setDuration(1000);
    blueAnimation->setEasingCurve(QEasingCurve::InOutCubic);
    this->colorRGBAnimation->addAnimation(redAnimation);
    this->colorRGBAnimation->addAnimation(greenAnimation);
    this->colorRGBAnimation->addAnimation(blueAnimation);

    this->colorAlphaAnimation = new QVariantAnimation();
    this->colorAlphaAnimation->setStartValue(50);
    this->colorAlphaAnimation->setEndValue(50);
    this->colorAlphaAnimation->setKeyValueAt(0.5, 100);
    this->colorAlphaAnimation->setEasingCurve(QEasingCurve::InExpo);
    connect(this->colorAlphaAnimation, SIGNAL(finished()), this, SLOT(alphaAnimationFinished()));

    connect(this->colorRGBAnimation, SIGNAL(valueChanged(QVariant)), this, SLOT(backgroundColorChanged(QVariant)));
    connect(this->colorAlphaAnimation, SIGNAL(valueChanged(QVariant)), this, SLOT(backgroundColorChanged(QVariant)));


    widgetChangeTimer->start(5000);

    createFakeDataTimer = new QTimer(this);
    connect(createFakeDataTimer, SIGNAL(timeout()), this, SLOT(fakeData()));
    createFakeDataTimer->start(500);

    QColor * color = new QColor(this->inrangeWhite, this->inrangeWhite, this->inrangeWhite, 50);
    this->setWidgetBackgroundColor(color);

    QPixmap p = QPixmap (":/image/icon/delete");
    int w = 25;
    int h = 25;

    // set a scaled pixmap to a w x h window keeping its aspect ratio
    ui->button_close->setIcon(p.scaled(w,h,Qt::KeepAspectRatio));
    //ui->label_close->hide();

    this->setMouseTracking(true);


    this->slotGetSensorListFinished(lstSensor);
}

void FloatingWorkout::setWidgetBackgroundColor(QColor *color){
    ui->wid_1_minimalistHr->setBackgroundColor(color);
    ui->wid_2_minimalistPower->setBackgroundColor(color);
    ui->wid_3_minimalistCadence->setBackgroundColor(color);
    ui->widget_workoutPlot->setBackgroundColor(color);
    ui->wid_2_workoutPlot_PowerZoom->setBackgroundColor(color);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::toggleTransparent() {

    if (isTransparent) {
        qDebug() << "SHOW NORMAL!";
        //revert back to normal QDialog, use saved stylesheet
        this->setStyleSheet(bakStylesheet);
        //        Qt::WindowFlags flags = Qt::Window;
        //        setWindowFlags(flags);
    }
    else {
        qDebug() << "SHOW TRANSPARENT!!";

        //save current stylesheet
        bakStylesheet = this->styleSheet();

        setStyleSheet("background:transparent;");
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::FramelessWindowHint); //this close the QDialog if not called in Constructor, why?.

        //        setWindowOpacity(0.5);
        //        ui->widget_time->setStyleSheet("background:transparent;");
        //        ui->widget_time->setAttribute(Qt::WA_TranslucentBackground);
        //        ui->widget_allSpeedo->setWindowOpacity(0.5);
    }
    isTransparent = !isTransparent;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::addToControlList(int antID, int fromHubNumber) {

    qDebug() << "Asking Workout Dialog for control" << "antID" << antID << "fromHubNumber" << fromHubNumber;

    if (!hashControlList.contains(antID)) {
        qDebug() << "Ok antID" << antID << "not in my List, you can control it!" << fromHubNumber;
        hashControlList.insert(antID, fromHubNumber);
        //emit signal back to Hub to let him add this ID
        emit permissionGrantedControl(antID, fromHubNumber);
    }
    /// DO NOT USE, because sending duplicate cause transfer to fail
    //    else if (hashControlList.size() >= nbTotalFecTrainer) {
    //        qDebug() << "OK we already control all trainer, we can control more for better reception!";
    //        hashControlList.insert(antID, fromHubNumber);
    //        emit permissionGrantedControl(antID, fromHubNumber);
    //    }
    else {
        qDebug() << "Sorry this trainer is already being controlled" <<  antID;
    }

}



//------------------------------------------------------------------------------------------------------------
void FloatingWorkout::batteryStatusReceived(QString sensorType, int level, int antID) {

    qDebug() << "batteryStatusReceived" << sensorType << "level:" << level;

    labelBatteryStatus->setVisible(true);

    QString levelStr;
    if (level == 0)
        levelStr = tr("critical");
    else  //1
        levelStr = tr("low");

    labelBatteryStatus->setText(tr("Battery Warning: ") + sensorType  + "(ID: " + QString::number(antID) + tr(") Battery is ") + levelStr);
    labelBatteryStatus->fadeInAndFadeOutAfterPause(400, 1000, 15000);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::initUI() {


    /// Widgets
    //showHeartRateDisplayWidget(account->display_hr);
    //showPowerDisplayWidget(account->display_power);
    //showCadenceDisplayWidget(account->display_cadence);

    /// Main plot Target and curve
    mainPlot->showHideSeperator(account->show_seperator_interval);
    mainPlot->showHideGrid(account->show_grid);
    mainPlot->showHideTargetCadence(account->show_cadence_target);
    mainPlot->showHideTargetHr(account->show_hr_target);
    mainPlot->showHideTargetPower(account->show_power_target);
    mainPlot->showHideCurveCadence(account->show_cadence_curve);
    mainPlot->showHideCurveHeartRate(account->show_hr_curve);
    mainPlot->showHideCurvePower(account->show_power_curve);
    mainPlot->showHideCurveSpeed(account->show_speed_curve);

    //if (workout.getWorkoutNameEnum() == Workout::MAP_TEST || workout.getWorkoutNameEnum() == Workout::OPEN_RIDE)
    //    ui->widget_workoutPlot->setSpinBoxDisabled();


    /// First Interval
    if (workout.getWorkoutNameEnum() != Workout::OPEN_RIDE) {

        timeWorkoutRemaining = workout.getDurationQTime();

        Interval firstInterval = workout.getInterval(0);
        timeInterval = firstInterval.getDurationQTime();

        adjustTargets(firstInterval);
    }
    else {
        // open ride
        timeWorkoutRemaining = timeInterval = QTime(0,0,0,0);

        targetPowerChanged_f(-1, 30);
        targetCadenceChanged_f(-1, 20);
        targetHrChanged_f(-1, 20);
    }
    ui->wid_2_workoutPlot_PowerZoom->setPosition(0);

    setMessagePlot();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::updatePausedTime(double totalTimePaused_msec) {

    this->totalTimePausedWorkout_msec = static_cast<int>(totalTimePaused_msec);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::updateRealTimeGraph(double totalTimeElapsed_msec) {

    timeElapsed_sec = totalTimeElapsed_msec /1000;

    if (!account->enable_studio_mode) {
        ui->wid_2_workoutPlot_PowerZoom->moveIntervalTime(totalTimeElapsed_msec);
    }
}


// For MAP Test, check last second average, if it's under the target for more than 20secs total or 10 sec consecutive, MAP test over
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::checkMAPTestOver() {

    qDebug() << "checkMAPTestOver!";


    bool fadeIn = false;
    //(ignore 3 first and 3 last second of Intervals with soundsActive check)
    if (soundsActive && averagePower1sec.at(0) < currentTargetPower-currentTargetPowerRange) {
        totalSecOffTargetInInterval++;
        totalConsecutiveOffTarget++;
        fadeIn = true;
    }
    else {
        totalConsecutiveOffTarget = 0;
    }

    QString redFontStart  = "<font color=\"red\">";
    QString fontEnd = "</font>";

    QString textTotalOffTargetInterval = tr("Total remaining: ")  + QString::number(20-totalSecOffTargetInInterval) + " sec.";
    if (totalSecOffTargetInInterval >= 15)
        textTotalOffTargetInterval = tr("Total remaining: ")+ redFontStart + QString::number(20-totalSecOffTargetInInterval) + " sec." + fontEnd;

    QString textConsecutiveOffTarget =  tr("Consecutive remaining: ") + QString::number(10-totalConsecutiveOffTarget) + " sec.";
    if (totalConsecutiveOffTarget >= 5)
        textConsecutiveOffTarget = tr("Consecutive remaining: ")+ redFontStart + QString::number(10-totalConsecutiveOffTarget) + " sec." + fontEnd;

    //Update Time remaining off target (top Left of graph)
    mainPlot->setAlertMessage(fadeIn, false, tr("MAP Interval ") + "#" + QString::number(currMAPInterval) + " - " + QString::number(currentTargetPower) + " watts"
                              + "<div style='color:white;height:7px;'>---------------------------------</div><br/> " + textTotalOffTargetInterval +
                              + "<br/>" + textConsecutiveOffTarget, 500);
    //Check if test interval over
    if (totalSecOffTargetInInterval >= 20 || totalConsecutiveOffTarget >= 10) {

        int secCompletedInInterval = static_cast<int>(180 - Util::convertQTimeToSecD(timeInterval));
        double mapResult = (currentTargetPower - 30) + (secCompletedInInterval/180.0 * currentTargetPower/10.0);
        mainPlot->setAlertMessage(true, false, workout.getName() + tr(" Result")
                                  + "<div style='color:white;height:7px;'>-------------------</div><br/> "  + QString::number(mapResult, 'f', 1)  + " watts", 500);
        //Go to cooldown interval (last interval)
        moveToInterval(workout.getNbInterval()-1, -1, -1, false);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::update1sec(double totalTimeElapsed_sec) {

    //    qDebug() << "#Update 1sec" << totalTimeElapsed_sec;
    int totalTimeElapsed_sec_i = static_cast<int>(totalTimeElapsed_sec);

    // Send last second data to DataWorkout, at second 2, send data received between [second 1 - second 2]
    sendLastSecondData(totalTimeElapsed_sec_i);

    // Check MAP Test stop condition
    if (!account->enable_studio_mode && workout.getWorkoutNameEnum() == Workout::MAP_TEST && currentIntervalObj.isTestInterval() && static_cast<int>(averagePower1sec.at(0)) != -1) {
        checkMAPTestOver();
    }


    // Reset mean 1 sec
    averageHr1sec.fill(-1);
    averageCadence1sec.fill(-1);
    averageSpeed1sec.fill(-1);
    averagePower1sec.fill(-1);

    nbPointHr1sec.fill(0);
    nbPointCadence1sec.fill(0);
    nbPointSpeed1sec.fill(0);
    nbPointPower1sec.fill(0);

    //temp
    for (int i=0; i<constants::nbMaxUserStudio; i++) {
        arrNbPointPower[i] = 0;
    }


    avgRightPedal1sec.fill(-1);
    avgLeftTorqueEff.fill(-1);
    avgRightTorqueEff.fill(-1);
    avgLeftPedalSmooth.fill(-1);
    avgRightPedalSmooth.fill(-1);
    avgCombinedPedalSmooth.fill(-1);
    avgSaturatedHemoglobinPercent1sec.fill(-1);
    avgTotalHemoglobinConc1sec.fill(-1);

    // Update the graph 'dark' area
    ui->widget_workoutPlot->updateMarkerTimeNow(totalTimeElapsed_sec);

    // Update minutes rode after 60secs
    nbUpdate1Sec++ ;
    if (nbUpdate1Sec == 60) {
        nbUpdate1Sec = 0;
    }

    // Update Timers
    timeElapsedTotal = timeElapsedTotal.addSecs(1);

    if (workout.getWorkoutNameEnum() == Workout::OPEN_RIDE) {
        timeInterval = timeInterval.addSecs(1);
    }
    else {
        timeInterval = timeInterval.addSecs(-1);
        timeWorkoutRemaining = timeWorkoutRemaining.addSecs(-1);

    }

    // Early exit
    if (workout.getWorkoutNameEnum() == Workout::OPEN_RIDE) {
        sendSlopes(0);
        return;
    }


    // To show next interval length instead of 0:00 in last second of an interval
    if (changeIntervalDisplayNextSecond) {
        moveToNextInterval();
        //if mamp test, insert interval until failure
        if (workout.getWorkoutNameEnum() == Workout::MAP_TEST && currentIntervalObj.isTestInterval())   //22 interval, start test interval
            insertInterval();
    }


    // Calculate new target
    if (!ignoreCondition) {
        Interval currInterval = workout.getLstInterval().at(currentInterval);
        double newTargetPower = calculateNewTargetPower();
        int range = currInterval.getFTP_range();
        targetPowerChanged_f(newTargetPower, range);

        int newTargetCadence2 = calculateNewTargetCadence();
        int range2 = currInterval.getCadence_range();
        targetCadenceChanged_f(newTargetCadence2, range2);

        double newTargetHr = calculateNewTargetHr();
        int range3 = currInterval.getHR_range();
        targetHrChanged_f(newTargetHr, range3);
    }



    if(timeInterval.minute()==0 && timeInterval.hour()== 0 && (timeInterval.second()==account->getNb_sec_show_interval_before() || timeInterval.second()==3 || timeInterval.second()==2 || timeInterval.second()==1) )  {
        soundsActive = false;
        if (currentInterval+1 != this->workout.getNbInterval()) {

            if (timeInterval.second() == account->getNb_sec_show_interval_before()) {
                //show next interval message
                Interval newInterval = workout.getLstInterval().at(currentInterval+1);
                if (newInterval.getDisplayMessage() != "")
                    ui->widget_workoutPlot->setDisplayIntervalMessage(true, tr("Next Interval: ") + newInterval.getDisplayMessage(), account->getNb_sec_show_interval());
            }
            else if (timeInterval.second()==3 || timeInterval.second()==2 || timeInterval.second()==1) {
                if (account->enable_sound && account->sound_interval)
                    soundPlayer->playSoundFirstBeepInterval();
                if (timeInterval.second()==1 && (currentInterval+1 < this->workout.getNbInterval()) ) {
                    changeIntervalDisplayNextSecond = true;
                }
            }
        }
    }
    else if(timeInterval.second()==0 && timeInterval.minute()==0 && timeInterval.hour()== 0) {

        //calculate pausedTime
        int intervalPausedTime_msec = totalTimePausedWorkout_msec - lastIntervalTotalTimePausedWorkout_msec;
        changeIntervalsDataWorkout(lastIntervalEndTime_msec, totalTimeElapsed_sec, intervalPausedTime_msec, false, currentIntervalObj.isTestInterval());


        timerCheckToActivateSound->start();
        currentInterval++;

        if (workout.getWorkoutNameEnum() == Workout::MAP_TEST && currentIntervalObj.isTestInterval()) {
            totalSecOffTargetInInterval = 0;
            currMAPInterval++;
        }


        // Workout over?
        if ( currentInterval >= workout.getNbInterval() ) {
            emit pauseClock();
            qDebug()<< "GOT HERE CHECK #3 WORKOUT OVER!!!**";
            workoutOver();
            ui->wid_2_workoutPlot_PowerZoom->setPosition(Util::convertQTimeToSecD(workout.getDurationQTime())*1000);
            return;
        }

        if (account->enable_sound && account->sound_interval)
            soundPlayer->playSoundLastBeepInterval();

        currentIntervalObj = workout.getInterval(currentInterval);
        timeInterval = currentIntervalObj.getDurationQTime();
    }


    ignoreCondition = false;
}



//--------------------------------------------------------------------------------------------------------------
void FloatingWorkout::lapButtonPressed() {

    qDebug() << "LAP BUTTON PRESSED" << timeElapsed_sec;

    if (isTestWorkout) {
        qDebug() << "Test workout, cant do laps!";
        return;
    }

    int intervalPausedTime_msec = totalTimePausedWorkout_msec - lastIntervalTotalTimePausedWorkout_msec;
    changeIntervalsDataWorkout(lastIntervalEndTime_msec, timeElapsed_sec, intervalPausedTime_msec, false, false);

    ui->widget_workoutPlot->addMarkerInterval(timeElapsed_sec);
    ui->wid_2_workoutPlot_PowerZoom->addMarkerInterval(timeElapsed_sec);
    ui->widget_workoutPlot->replot();

    if (workout.getWorkoutNameEnum() == Workout::OPEN_RIDE) {
        timeInterval = QTime(0,0,0,0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::adjustWorkoutDifficulty(int percentageIncrease) {

    double diffFromActualDifficulty = percentageIncrease - currentWorkoutDifficultyPercentage;
    diffFromActualDifficulty = diffFromActualDifficulty/100.0;

    //    qDebug() << "ADJUST WORKOUT DIFFICULTY!!" << percentageIncrease << "diff100: " << diffFromActualDifficulty;

    // pause workout if not paused
    if (!isWorkoutPaused) {
        start_or_pause_workout();
    }

    // Compute new workout
    QList<Interval> lstIntervalAdjusted;

    foreach (Interval interval, workout.getLstInterval()) {

        if (interval.getPowerStepType() != Interval::NONE) {
            //do not check negative, could change progressive interval to flat if so..
            interval.setTargetFTP_start(interval.getFTP_start() + diffFromActualDifficulty);
            interval.setTargetFTP_end(interval.getFTP_end() + diffFromActualDifficulty);
        }
        if (interval.getHRStepType() != Interval::NONE) {

            interval.setTargetHR_start(interval.getHR_start() + diffFromActualDifficulty);
            interval.setTargetHR_end(interval.getHR_end() + diffFromActualDifficulty);
        }
        lstIntervalAdjusted.append(interval);
    }


    Workout workoutEdited(workout.getFilePath(), workout.getWorkoutNameEnum(), lstIntervalAdjusted,
                          workout.getName(), workout.getCreatedBy(), workout.getDescription(), workout.getPlan(), workout.getType());
    this->workout = workoutEdited;

    // refresh view
    ui->widget_workoutPlot->setWorkoutData(workout, false);
    ui->wid_2_workoutPlot_PowerZoom->setWorkoutData(workout, WorkoutPlotZoomer::POWER, false);

    //adjust mini-graph and widget to new target
    currentIntervalObj = workout.getInterval(currentInterval);
    adjustTargets(currentIntervalObj);

    currentWorkoutDifficultyPercentage = percentageIncrease;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::insertInterval() {

    qDebug() << "INSERT INTERVAL!";

    //add parameter function
    int whereToInsert = currentInterval+2;


    // pause workout if not paused [to remove, should be done in real time]
    //    if (!isWorkoutPaused) {
    //        start_or_pause_workout();
    //    }

    // Compute new workout
    QList<Interval> lstIntervalModified = workout.getLstInterval();

    // for Mamp test, take next interval and increase 30W
    Interval nextInterval = lstIntervalModified.at(currentInterval+1);
    double currentWattsLevel = nextInterval.getFTP_start() * account->getFTP();
    double nextWattsLevel = currentWattsLevel + 30;
    double inFTP = nextWattsLevel/account->getFTP();

    QString msgNextInterval = QString::number(nextWattsLevel) + " watts";
    nextInterval.setTargetFTP_start(inFTP);
    nextInterval.setDisplayMsg(msgNextInterval);

    lstIntervalModified.insert(whereToInsert, nextInterval);

    Workout workoutEdited(workout.getFilePath(), workout.getWorkoutNameEnum(), lstIntervalModified,
                          workout.getName(), workout.getCreatedBy(), workout.getDescription(), workout.getPlan(), workout.getType());
    this->workout = workoutEdited;

    // refresh view
    ui->widget_workoutPlot->setWorkoutData(workout, false);
    ui->wid_2_workoutPlot_PowerZoom->setWorkoutData(workout, WorkoutPlotZoomer::POWER, false);

    ui->widget_workoutPlot->updateMarkerTimeNow(timeElapsed_sec);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::moveToInterval(int nbInterval, double secWorkout, double startIntervalSec, bool showConfirmation) {

    //clear focus on QwtPlot for hotkeys needs focus on this QDialog
    this->setFocus();

    // Not legal for test workout or Open Ride (except Manual = showConfirmation at false)
    if (showConfirmation && (bignoreClickPlot || isTestWorkout  || workout.getWorkoutNameEnum() == Workout::OPEN_RIDE) )
        return;

    qDebug() << "Workout Dialog, move to interval" << nbInterval << "sec:" << secWorkout << "startIntervalSec" << startIntervalSec;

    int nbIntervalToDelete = 0;
    if (currentInterval >= nbInterval)
        return;
    else {
        nbIntervalToDelete = nbInterval - currentInterval -1;
    }
    qDebug() << "we have to delete " << nbIntervalToDelete << "interval";

    // pause workout if not paused and manually clicked
    if (!isWorkoutPaused && showConfirmation) {
        start_or_pause_workout();
    }

    QString timeStartInterval = Util::showQTimeAsString(Util::convertMinutesToQTime(startIntervalSec/60.0));

    //ask confirmation
    if (showConfirmation) {
        isAskingUserQuestion = true;
        QMessageBox msgBox(this);
        msgBox.setStyleSheet("QLabel {color: black;}");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("Move to the interval starting at: ") + timeStartInterval + "?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) {
            isAskingUserQuestion = false;
            return;
        }
        isAskingUserQuestion = false;
    }


    //Calculate timeDone in currentInterval
    double currentIntervalTimeDone =  timeElapsed_sec - lastIntervalEndTime_sec;
    double timeSkippedInInterval = Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime()) - currentIntervalTimeDone;
    lastIntervalEndTime_sec += currentIntervalTimeDone;
    qDebug() << "We did:" << currentIntervalTimeDone << " of the current interval, we skipped:" << timeSkippedInInterval;

    // Compute new workout and new interval
    QList<Interval> copyLstInterval = workout.getLstInterval();
    Interval intervalToModify = currentIntervalObj;

    // Update time interval
    QTime timeActuallyDone(0,0,0,0);
    timeActuallyDone = timeActuallyDone.addMSecs(static_cast<int>(currentIntervalTimeDone*1000));
    intervalToModify.setTime(timeActuallyDone);
    qDebug() << "OLD INTERVAL TIME WAS:" << currentIntervalObj.getDurationQTime() << " NOW IT'S:" << intervalToModify.getDurationQTime();

    // Adjust interval target (for drawing purpose only)
    if (intervalToModify.getPowerStepType() == Interval::PROGRESSIVE) {
        double totalSec = Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime());
        /// y=ax+b
        double b = currentIntervalObj.getFTP_start();
        double a = (currentIntervalObj.getFTP_end() - b) / totalSec;
        double x = currentIntervalTimeDone;
        double y = a*x + b;
        intervalToModify.setTargetFTP_end(y);
    }
    if (intervalToModify.getCadenceStepType() == Interval::PROGRESSIVE) {
        double totalSec = Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime());
        /// y=ax+b
        double b = currentIntervalObj.getCadence_start();
        double a = (currentIntervalObj.getCadence_end() - b) / totalSec;
        double x = currentIntervalTimeDone;
        double y = a*x + b;
        intervalToModify.setTargetCadence_end(static_cast<int>(y));
    }
    if (intervalToModify.getHRStepType() == Interval::PROGRESSIVE) {
        double totalSec = Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime());
        /// y=ax+b
        double b = currentIntervalObj.getHR_start();
        double a = (currentIntervalObj.getHR_end() - b) / totalSec;
        double x = currentIntervalTimeDone;
        double y = a*x + b;
        intervalToModify.setTargetHR_end(y);
    }
    copyLstInterval.replace(currentInterval, intervalToModify);


    //Remove interval that we skipped (if clicked more than 1 interval ahead)
    for (int i=0; i<nbIntervalToDelete; i++) {
        copyLstInterval.removeAt(currentInterval+1);
    }

    Workout workoutEdited(workout.getFilePath(), workout.getWorkoutNameEnum(), copyLstInterval,
                          workout.getName(), workout.getCreatedBy(), workout.getDescription(), workout.getPlan(), workout.getType());


    this->workout = workoutEdited;

    // refresh view
    ui->widget_workoutPlot->setWorkoutData(workout, false);
    ui->wid_2_workoutPlot_PowerZoom->setWorkoutData(workout, WorkoutPlotZoomer::POWER, false);


    //Create a lap for FIT FIle
    if (timeElapsed_sec - lastIntervalEndTime_msec > 1) {
        int intervalPausedTime_msec = totalTimePausedWorkout_msec - lastIntervalTotalTimePausedWorkout_msec;
        changeIntervalsDataWorkout(lastIntervalEndTime_msec, timeElapsed_sec, intervalPausedTime_msec, false, currentIntervalObj.isTestInterval());
    }

    //Go to selected interval
    currentInterval++;
    currentIntervalObj = workout.getInterval(currentInterval);
    adjustTargets(currentIntervalObj);

    if (currentIntervalObj.getDisplayMessage() != "")
        ui->widget_workoutPlot->setDisplayIntervalMessage(false, currentIntervalObj.getDisplayMessage(), account->getNb_sec_show_interval());


    //-- Update Timers
    timeInterval = currentIntervalObj.getDurationQTime();
    qDebug() << "NEXT INTERVAL IS LONG :" << timeInterval;

    //calculate workoutRemainingTime
    timeWorkoutRemaining = QTime(0,0,0,0);
    for (int i=currentInterval; i<workout.getLstInterval().size(); i++) {
        timeWorkoutRemaining = timeWorkoutRemaining.addSecs(static_cast<int>(Util::convertQTimeToSecD(workout.getInterval(i).getDurationQTime())));
    }

    //start timer ignore click (click on QDialog response trigger a new click event)
    bignoreClickPlot = true;
    timerIgnoreClickPlot->start(500);
}


//--------------------------------------------------------------------------------------------------------------
void FloatingWorkout::moveToNextInterval() {

    qDebug() << "moveToNextInterval";

    int timeToAdd = static_cast<int>(Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime()));
    lastIntervalEndTime_sec += timeToAdd;
    qDebug() << "ok adding " << timeToAdd << "to lastIntervalEndTime is now:" << lastIntervalEndTime_sec;

    changeIntervalDisplayNextSecond = false;
    ignoreCondition = true;


    Interval newInterval = workout.getLstInterval().at(currentInterval+1);
    if (newInterval.getDisplayMessage() != "")
        ui->widget_workoutPlot->setDisplayIntervalMessage(false, newInterval.getDisplayMessage(), account->getNb_sec_show_interval());


    adjustTargets(newInterval);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::startWorkout() {


    emit startClock();

    QDateTime dateTimeStartedWorkout;
    dateTimeStartedWorkout = QDateTime::currentDateTime();

    if (account->enable_studio_mode) {
        for (int i=0; i<account->nb_user_studio; i++) {
            arrDataWorkout[i]->setStartTimeWorkout(dateTimeStartedWorkout.toUTC());
            UserStudio userStudio = vecUserStudio.at(i);
            QString userIdentifier = "user" + QString::number(i+1) + "-" + Util::cleanForOsSaving(userStudio.getDisplayName());
            arrDataWorkout[i]->initFitFile(true, userIdentifier, workout.getName(), dateTimeStartedWorkout.toUTC() );
        }
    }
    else {
        arrDataWorkout[0]->setStartTimeWorkout(dateTimeStartedWorkout.toUTC());
        arrDataWorkout[0]->initFitFile(false, account->email_clean, workout.getName(), dateTimeStartedWorkout.toUTC() );
    }

    timerCheckToActivateSound->start();
}




////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::workoutOver() {

    isWorkoutOver = true;

    emit sendOxygenCommand(sensorOxygen.getAntId(), Oxygen_Controller::STOP_SESSION);

    qDebug() << "STOPPING WORKOUT";
    if (account->enable_sound && account->sound_end_workout)
        soundPlayer->playSoundEndWorkout();

//    ui->widget_workoutPlot->setMessageEndWorkout();
    ui->widget_workoutPlot->setDisplayIntervalMessage(true, tr("Workout Completed!"), 20000);

    isWorkoutPaused = true;
    //ui->widget_workoutPlot->setSpinBoxDisabled();


    setWidgetsStopped(true);


    //Close FIT FILE
    closeFitFiles(timeElapsed_sec);


    // Show Test Result?
    showTestResult();



    // Set workout to done
    account->insertHashWorkoutDone(workout.getName());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::start_or_pause_workout() {


    qDebug() << "start_or_pause_workout!";


    if (isWorkoutOver) {
        return;
    }

    // If workout not started, we start it
    if (!isWorkoutStarted) {
        if (account->enable_sound  && account->sound_pause_resume_workout)
            soundPlayer->playSoundStartWorkout();
        ui->widget_workoutPlot->removeMainMessage();
        isWorkoutStarted = true;
        isWorkoutPaused = false;
        if (this->workout.getInterval(0).getDisplayMessage() != "")
            ui->widget_workoutPlot->setDisplayIntervalMessage(true, this->workout.getInterval(0).getDisplayMessage(), account->getNb_sec_show_interval());
        setWidgetsStopped(false);
        startWorkout();
        emit playPlayer();
        emit sendOxygenCommand(sensorOxygen.getAntId(), Oxygen_Controller::START_SESSION);

    }
    // If workout paused, we resume it
    else if (isWorkoutStarted && isWorkoutPaused) {
        qDebug() << "RESUME WORKOUT!!!";
        if (account->enable_sound  && account->sound_pause_resume_workout)
            soundPlayer->playSoundStartWorkout();
        ui->widget_workoutPlot->removeMainMessage();
        isWorkoutPaused = false;

        setWidgetsStopped(false);
        emit resumeClock();
        emit playPlayer();

    }
    // If not paused, we pause it
    else if (isWorkoutStarted && !isWorkoutPaused) {
        qDebug() << "PAUSE WORKOUT!!!";
        if (account->enable_sound  && account->sound_pause_resume_workout)
            soundPlayer->playSoundPauseWorkout();
        isWorkoutPaused = true;
        setWidgetsStopped(true);
        setMessagePlot();
        emit pauseClock();
        emit pausePlayer();
    }
}




//--------------------------------------------------------------------------------------------------
void FloatingWorkout::sendLastSecondData(int seconds) {

    if (account->enable_studio_mode) {
        for (int i=0; i<account->nb_user_studio; i++) {
            //            qDebug() << "Update Data Workout, User: " << i <<   "HR:" << averageHr1sec.at(i) << "CAD:" << averageCadence1sec.at(i) << "Speed:" << averageSpeed1sec.at(i) << "Power:" << averagePower1sec.at(i) <<
            //                        "rightPedal:";
            arrDataWorkout[i]->updateData(account->enable_studio_mode, seconds, averageHr1sec.at(i), averageCadence1sec.at(i), averageSpeed1sec.at(i), averagePower1sec.at(i),
                                          avgRightPedal1sec.at(i), avgLeftTorqueEff.at(i), avgRightTorqueEff.at(i), avgLeftPedalSmooth.at(i), avgRightPedalSmooth.at(i), avgCombinedPedalSmooth.at(i),
                                          avgSaturatedHemoglobinPercent1sec.at(i), avgTotalHemoglobinConc1sec.at(i));
        }
    }
    else {
        arrDataWorkout[0]->updateData(account->enable_studio_mode, seconds, averageHr1sec.at(0), averageCadence1sec.at(0), averageSpeed1sec.at(0), averagePower1sec.at(0),
                                      avgRightPedal1sec.at(0), avgLeftTorqueEff.at(0), avgRightTorqueEff.at(0), avgLeftPedalSmooth.at(0), avgRightPedalSmooth.at(0), avgCombinedPedalSmooth.at(0),
                                      avgSaturatedHemoglobinPercent1sec.at(0), avgTotalHemoglobinConc1sec.at(0));
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::HrDataReceived(int userID, int value) {


    // invalid value, show "-" to the user
    if (value == -1) {
        //ui->wid_1_minimalistHr->setValue(value);
        return;
    }
    if (value < 0)
        return;


    // Mark pairing as done
    if (!account->enable_studio_mode && !hrPairingDone) {
        hrPairingDone = true;
        labelPairHr->setText(labelPairHr->text() + msgPairingDone);
        labelPairHr->setStyleSheet("background-color : rgba(1,1,1,220); color : green;");
        labelPairHr->fadeOut(500);
        checkPairingCompleted();
    }


    if (!isWorkoutPaused && !isWorkoutOver) {

        arrDataWorkout[userID-1]->checkUpdateMaxHr(value);

        //averaging 1sec, TODO, check with userID - at(0) replace with userID
        if (nbPointHr1sec.at(userID-1) == 0) {
            averageHr1sec.replace(userID-1, value);
        }
        else {
            int nbPoint = nbPointHr1sec.at(userID-1);
            double firstEle = averageHr1sec.at(userID-1) *(static_cast<double>(nbPoint)/(nbPoint+1));
            double secondEle = (static_cast<double>(value))/(nbPoint+1);
            averageHr1sec.replace(userID-1, firstEle + secondEle);
        }
        nbPointHr1sec.replace(userID-1, nbPointHr1sec.at(userID-1) + 1);


        //Update Graph (zoomer)
    }

    // Show data to the display
    ui->wid_1_minimalistHr->setValue(value);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::CadenceDataReceived(int userID, int value) {
    // invalid value, show "-" to the user
    if (value == -1 || value > 250) {
        //ui->wid_3_minimalistCadence->setValue(value);
        return;
    }
    if (value < 0)
        return;


    // Mark pairing as done
    if (!account->enable_studio_mode && !cadencePairingDone) {
        cadencePairingDone = true;
        labelCadence->setText(labelCadence->text() + msgPairingDone);
        labelCadence->setStyleSheet("background-color : rgba(1,1,1,220); color : green;");
        checkPairingCompleted();
    }
    if (!account->enable_studio_mode && !scPairingDone) {
        scPairingDone = true;
        labelSpeedCadence->setText(labelSpeedCadence->text() + msgPairingDone);
        labelSpeedCadence->setStyleSheet("background-color : rgba(1,1,1,220); color : green;");
        checkPairingCompleted();
    }


    if (!isWorkoutPaused && !isWorkoutOver) {

        arrDataWorkout[userID-1]->checkUpdateMaxCad(value);

        ///Below Pause threshold?
        if (!account->enable_studio_mode && (account->start_trigger == 0) && (value < account->value_cadence_start) ) {
            start_or_pause_workout();
        }

        ///Check to play sound, 10sec cooldown between sound and not first/last 3 second of an interval
        if (!account->enable_studio_mode && currentTargetCadence != -1 && soundsActive && account->enable_sound) {
            /// TOO LOW
            if (account->sound_alert_cadence_under_target && (value < currentTargetCadence - currentTargetCadenceRange) && soundCadenceTooLowActive) {
                soundPlayer->playSoundCadenceTooLow();
                soundCadenceTooLowActive = false;
                timerCheckReactivateSoundCadenceTooLow->start();
            }
            /// TOO HIGH
            else if (account->sound_alert_cadence_above_target && (value > currentTargetCadence + currentTargetCadenceRange) && soundCadenceTooHighActive) {
                soundPlayer->playSoundCadenceTooHigh();
                soundCadenceTooHighActive = false;
                timerCheckReactivateSoundCadenceTooHigh->start();
            }
        }


        //averaging 1sec
        if (nbPointCadence1sec.at(userID-1) == 0) {
            averageCadence1sec.replace(userID-1, value);
        }
        else {
            int nbPoint = nbPointCadence1sec.at(userID-1);
            double firstEle = averageCadence1sec.at(userID-1) *(static_cast<double>(nbPoint)/(nbPoint+1));
            double secondEle = (static_cast<double>(value))/(nbPoint+1);
            averageCadence1sec.replace(userID-1, firstEle + secondEle);
        }
        nbPointCadence1sec.replace(userID-1, nbPointCadence1sec.at(userID-1) + 1);
    } else if (!account->enable_studio_mode && (account->start_trigger == 0) && (!isWorkoutStarted || isWorkoutPaused) && (value > account->value_cadence_start) ) {
        qDebug() << "Above cadence threshold, workout start!";
        start_or_pause_workout();
    }





    // Show raw data to the display
    ui->wid_3_minimalistCadence->setValue(value);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::PowerDataReceived(int userID, int value) {

    // invalid value, show "-" to the user
    if (value == -1) {
        ui->wid_2_workoutPlot_PowerZoom->updateTextLabelValue(value);
        ui->wid_2_minimalistPower->setValue(value);
        return;
    }


    if (!account->enable_studio_mode)
        value = value + account->offset_power;
    if (value < 0)
        return;

    // Send power To Clock
    emit sendPowerData(userID, value);



    // Mark pairing as done
    if (!account->enable_studio_mode && !powerPairingDone) {
        powerPairingDone = true;
        labelPower->setText(labelPower->text() + msgPairingDone);
        labelPower->setStyleSheet("background-color : rgba(1,1,1,220); color : green;");
        checkPairingCompleted();
    }
    if (!account->enable_studio_mode && !fecPairingDone) {
        fecPairingDone = true;
        labelFEC->setText(labelFEC->text() + msgPairingDone);
        labelFEC->setStyleSheet("background-color : rgba(1,1,1,220); color : green;");
        checkPairingCompleted();
    }



    int rollingAverage = value;
    ///------- Averaging Power ------------------------------------------------------------------------------
    if (!isWorkoutPaused && !isWorkoutOver && account->averaging_power > 0) {

        ///For Testing ---------------------
        //        qDebug() << "Printing Queue Before Value:" << value;
        //        for (int i=0; i< arrQueuePower[userID-1].size(); i++) {
        //            qDebug() << "queue["<<i<<"]=" <<  arrQueuePower[userID-1].at(i);
        //        }


        // Get current second
        int currentSecondPower = static_cast<int>(timeElapsed_sec);
        // If the second changed, add new value to top of queue
        if (currentSecondPower != arrLastSecondPower[userID-1]) {
            arrQueuePower[userID-1].enqueue(value);
            // check the queue size, remove element if needed (loop: if settings just changed need to remove more than 1 value)
            while (arrQueuePower[userID-1].size() > account->averaging_power) {
                arrQueuePower[userID-1].dequeue();
            }
        }
        // If the second is the same, recalculate average and replace value of the tail
        else {
            ///first second workout
            if (arrQueuePower[userID-1].size() < 1) {
                arrQueuePower[userID-1].enqueue(value);
            }
            else {
                double firstEle = arrQueuePower[userID-1].last() *(static_cast<double>(arrNbPointPower[userID-1])/(arrNbPointPower[userID-1]+1));
                double secondEle = (static_cast<double>(value))/(arrNbPointPower[userID-1]+1);
                // replace last element of the queue
                double newAvg = firstEle + secondEle;
                if (newAvg > 0)
                    arrQueuePower[userID-1].replace( arrQueuePower[userID-1].size()-1,  newAvg);
            }
        }
        arrNbPointPower[userID-1]++;
        arrLastSecondPower[userID-1] = currentSecondPower;


        /// Get the average of the queue
        double avgQueue = 0.0;
        double totalQueue = 0.0;
        if (arrQueuePower[userID-1].size() > 0) {
            for (int i=0; i<arrQueuePower[userID-1].size(); i++) {
                totalQueue += arrQueuePower[userID-1].at(i);
            }
            avgQueue = totalQueue/arrQueuePower[userID-1].size();
            /// replace value with the rolling average
            if (avgQueue > 0)
                rollingAverage = qRound(avgQueue);
        }


        ///For Testing ---------------------
        //        qDebug() << "Printing Queue After Enqueue:";
        //        for (int i=0; i< arrQueuePower[userID-1].size(); i++) {
        //            qDebug() << "queue["<<i<<"]=" <<  arrQueuePower[userID-1].at(i);
        //        }
        //        qDebug() << "avg Queue is:" << avgQueue << "rollingAverage :" << rollingAverage;
    }
    /// -----------------------------------------------------------------------------------------------------




    if (!isWorkoutPaused && !isWorkoutOver) {

        arrDataWorkout[userID-1]->checkUpdateMaxPower(rollingAverage);

        ///Below Pause treshold?
        if (!account->enable_studio_mode && (account->start_trigger == 1) && (rollingAverage < account->value_power_start) ) {
            start_or_pause_workout();
        }

        ///Check to play sound, 10sec cooldown between sound and not first/last 3 second of an interval
        if (!account->enable_studio_mode && currentTargetPower != -1 && soundsActive && account->enable_sound) {
            /// TOO LOW
            if (account->sound_alert_power_under_target && (rollingAverage < currentTargetPower - currentTargetPowerRange) && soundPowerTooLowActive) {
                soundPlayer->playSoundPowerTooLow();
                soundPowerTooLowActive = false;
                timerCheckReactivateSoundPowerTooLow->start();
            }
            /// TOO HIGH
            else if (account->sound_alert_power_above_target && (rollingAverage > currentTargetPower + currentTargetPowerRange) && soundPowerTooHighActive) {
                soundPlayer->playSoundPowerTooHigh();
                soundPowerTooHighActive = false;
                timerCheckReactivateSoundPowerTooHigh->start();
            }
        }


        //averaging 1sec
        if (nbPointPower1sec.at(userID-1) == 0) {
            averagePower1sec.replace(userID-1, value);
        }
        else {
            int nbPoint = nbPointPower1sec.at(userID-1);
            double firstEle = averagePower1sec.at(userID-1) *(static_cast<double>(nbPoint)/(nbPoint+1));
            double secondEle = (static_cast<double>(value))/(nbPoint+1);
            averagePower1sec.replace(userID-1, firstEle + secondEle);
        }
        nbPointPower1sec.replace(userID-1, nbPointPower1sec.at(userID-1) + 1);

        //Update Graph (zoomer)
        ui->wid_2_workoutPlot_PowerZoom->updateCurve(timeElapsed_sec, rollingAverage);


    }
    ///Resume Workout?
    if (!account->enable_studio_mode && !isCalibrating && !isAskingUserQuestion && (account->start_trigger == 1) && isWorkoutPaused && !isWorkoutOver && (value >= account->value_power_start) ) {
        start_or_pause_workout();
    }


    // Show raw data to the display
    //ui->wid_2_workoutPlot_PowerZoom->updateTextLabelValue(rollingAverage);
    ui->wid_2_minimalistPower->setValue(rollingAverage);
}



void FloatingWorkout::pedalMetricReceived(int userID, double leftTorqueEff, double rightTorqueEff,
                                        double leftPedalSmooth, double rightPedalSmooth, double combinedPedalSmooth) {

    //    qDebug() << "FloatingWorkout::PEDALMETRIC Received. leftTorqueEff" << leftTorqueEff << "rightTorqueEff" << rightTorqueEff <<
    //                "leftPedalSmooth" << leftPedalSmooth << "rightPedalSmooth" << rightPedalSmooth << "combinedPedalSmooth" << combinedPedalSmooth;

    avgLeftTorqueEff.replace(userID-1, leftTorqueEff);
    avgRightTorqueEff.replace(userID-1, rightTorqueEff);
    avgLeftPedalSmooth.replace(userID-1, leftPedalSmooth);
    avgRightPedalSmooth.replace(userID-1, rightPedalSmooth);
    avgCombinedPedalSmooth.replace(userID-1, combinedPedalSmooth);

}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::sendTargetsPower(double percentageTarget, int range) {
    ui->wid_2_workoutPlot_PowerZoom->targetChanged(percentageTarget, range);
    ui->wid_2_minimalistPower->setTarget(percentageTarget, range);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::sendTargetsCadence(int target, int range) {
    ui->wid_3_minimalistCadence->setTarget(target, range);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::sendTargetsHr(double percentageTarget, int range) {
    ui->wid_1_minimalistHr->setTarget(percentageTarget, range);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::sendSlopes(double slope) {
    if (idFecMainUser != -1) {
        emit setSlope(idFecMainUser, slope);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::sendLoads(double percentageFTP) {
    if (idFecMainUser != -1){
        emit setLoad(idFecMainUser, currentTargetPower);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::targetPowerChanged_f(double percentageTarget, int range) {


    if (percentageTarget <= 0 || isUsingSlopeMode || !account->control_trainer_resistance) {
        sendSlopes(0);
    }
    else if(account->control_trainer_resistance) {
        sendLoads(percentageTarget);
    }


    currentTargetPower = qRound(percentageTarget * account->getFTP());
    currentTargetPowerRange =  range;
    sendTargetsPower(percentageTarget, range);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::targetCadenceChanged_f(int target, int range) {


    currentTargetCadence = target;
    currentTargetCadenceRange = range;

    sendTargetsCadence(target, range);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::targetHrChanged_f(double percentageTarget, int range) {

    sendTargetsHr(percentageTarget, range);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::adjustTargets(Interval interval) {

    if (interval.isTestInterval() || interval.getPowerStepType() == Interval::NONE || workout.getWorkoutNameEnum() == Workout::OPEN_RIDE)
        isUsingSlopeMode = true;
    else
        isUsingSlopeMode = false;

    ///Power
    if (interval.getPowerStepType() != Interval::NONE) {
        targetPowerChanged_f(interval.getFTP_start(), interval.getFTP_range());
    }
    else
        targetPowerChanged_f(-1, currentIntervalObj.getFTP_range());
    //Cadence


    /// Target cadence
    if (interval.getCadenceStepType() != Interval::NONE)
        targetCadenceChanged_f(interval.getCadence_start(), interval.getCadence_range());
    else
        targetCadenceChanged_f(-1, interval.getCadence_range());


    /// Target hr
    if (interval.getHRStepType() != Interval::NONE)
        targetHrChanged_f(interval.getHR_start(), interval.getHR_range());
    else
        targetHrChanged_f(-1, interval.getHR_range());

}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double FloatingWorkout::calculateNewTargetPower() {

    /// Intervale de base
    if (currentIntervalObj.getPowerStepType() == Interval::FLAT) {
        return currentIntervalObj.getFTP_start();
    }
    else if(currentIntervalObj.getPowerStepType() == Interval::NONE) {
        return -1;
    }

    double totalSec = Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime());
    /// y=ax+b
    double b = currentIntervalObj.getFTP_start();
    double a = (currentIntervalObj.getFTP_end() - b) / totalSec;
    double x = totalSec - (Util::convertQTimeToSecD(timeInterval));
    double y = a*x + b;

    return y;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double FloatingWorkout::calculateNewTargetHr() {

    /// Intervale de base
    if (currentIntervalObj.getHRStepType() == Interval::FLAT) {
        return currentIntervalObj.getHR_start();
    }
    else if(currentIntervalObj.getHRStepType() == Interval::NONE) {
        return -1;
    }

    double totalSec = Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime());
    /// y=ax+b
    double b = currentIntervalObj.getHR_start();
    double a = (currentIntervalObj.getHR_end() - b) / totalSec;
    double x = totalSec - (Util::convertQTimeToSecD(timeInterval));
    double y = a*x + b;

    return y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
int FloatingWorkout::calculateNewTargetCadence() {

    /// Intervale de base
    if (currentIntervalObj.getCadenceStepType() == Interval::FLAT) {
        return currentIntervalObj.getCadence_start();
    }
    else if(currentIntervalObj.getCadenceStepType() == Interval::NONE) {
        return -1;
    }

    double totalSec = Util::convertQTimeToSecD(currentIntervalObj.getDurationQTime());
    /// y=ax+b
    double b = currentIntervalObj.getCadence_start();
    double a = (currentIntervalObj.getCadence_end() - b) / totalSec;
    double x = totalSec - (Util::convertQTimeToSecD(timeInterval));
    double y = a*x + b;

    return static_cast<int>(y);
}


///0 = Minimalist
///1 = Detailed
///2 = Graph
///3 = Hide
////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::showHeartRateDisplayWidget(int display) {

    if (account->show_hr_widget) {
        if (display == 0) { /// Minimalist
            ui->wid_1_minimalistHr->setVisible(true);
        }
        else if (display == 1) { /// Detailed
            ui->wid_1_minimalistHr->setVisible(false);
        }
        else if (display == 2) { /// Graph
            ui->wid_1_minimalistHr->setVisible(false);
        }
    }
    else { /// Hide
        ui->wid_1_minimalistHr->setVisible(false);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::showPowerDisplayWidget(int display) {

    if (account->show_power_widget) {
        if (display == 0) { /// Minimalist
            ui->wid_2_minimalistPower->setVisible(true);
            ui->wid_2_workoutPlot_PowerZoom->setVisible(false);
        }
        else if (display == 1) { /// Detailed
            ui->wid_2_minimalistPower->setVisible(false);
            ui->wid_2_workoutPlot_PowerZoom->setVisible(false);
        }
        else if (display == 2) { /// Graph
            ui->wid_2_minimalistPower->setVisible(false);
            ui->wid_2_workoutPlot_PowerZoom->setVisible(true);
        }
        else if (display == 3) { ///Graph and Detailed
            ui->wid_2_minimalistPower->setVisible(false);
            ui->wid_2_workoutPlot_PowerZoom->setVisible(true);
        }
    }
    else { /// Hide
        ui->wid_2_minimalistPower->setVisible(false);
        ui->wid_2_workoutPlot_PowerZoom->setVisible(false);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::showCadenceDisplayWidget(int display) {

    if (account->show_cadence_widget) {
        if (display == 0) { /// Minimalist
            ui->wid_3_minimalistCadence->setVisible(true);
        }
        else if (display == 1) { /// Detailed
            ui->wid_3_minimalistCadence->setVisible(false);
        }
        else if (display == 2) { /// Graph
            ui->wid_3_minimalistCadence->setVisible(false);
        }
    }
    else { /// Hide
        ui->wid_3_minimalistCadence->setVisible(false);
    }
}


////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::setMessagePlot() {

    if (isWorkoutOver)
        return;


    bool showMessage = false; //only show if workout is paused or not started

    /// 0=Cadence 1=Power 2=Speed 3=Button
    int trigger = account->start_trigger;
    QString toShow;


    if (!isWorkoutStarted) {
        showMessage = true;

        if (account->enable_studio_mode) {
            toShow = tr("To start workout, press Enter or the start button");
        }
        else if (trigger == 0) {
            toShow = tr("To start workout, pedal over ");
            toShow += QString::number(account->value_cadence_start) + " ";
            toShow += tr("rpm");
        }
        else if (trigger == 1) {
            toShow = tr("To start workout, pedal over ");
            toShow += QString::number(account->value_power_start) + " ";
            toShow += tr("watts");
        }
        else if (trigger == 2) {
            toShow = tr("To start workout, pedal over ");
            toShow += QString::number(account->value_speed_start) + " ";
            if (account->distance_in_km)
                toShow += tr("km/h");
            else
                toShow += tr("mph");
        }
        else {
            toShow = tr("To start workout, press Enter or the start button");
        }
    }
    else if (isWorkoutPaused) {
        showMessage = true;

        if (account->enable_studio_mode) {
            toShow = tr("To resume workout, press Enter or the resume button");
        }
        else if (trigger == 0) {
            toShow = tr("To resume workout, pedal over ");
            toShow += QString::number(account->value_cadence_start) + " ";
            toShow += tr("rpm");
        }
        else if (trigger == 1) {
            toShow = tr("To resume workout, pedal over ");
            toShow += QString::number(account->value_power_start) + " ";
            toShow += tr("watts");
        }
        else if (trigger == 2) {
            toShow = tr("To resume workout, pedal over ");
            toShow += QString::number(account->value_speed_start) + " ";
            if (account->distance_in_km)
                toShow += tr("km/h");
            else
                toShow += tr("mph");
        }
        else {
            toShow = tr("To resume workout, press Enter or the resume button");
        }
    }
    if (showMessage) {
        ui->widget_workoutPlot->setMessage(toShow);
    }
}



////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::setWidgetsStopped(bool b) {

    qDebug() << "SetWidgetStopped!";

    ui->widget_workoutPlot->setStopped(b);
    ui->wid_2_workoutPlot_PowerZoom->setStopped(b);
    ui->wid_1_minimalistHr->setStopped(b);
    ui->wid_2_minimalistPower->setStopped(b);
    ui->wid_3_minimalistCadence->setStopped(b);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::slotGetSensorListFinished(QList<Sensor> lstSensor) {

    if (lstSensor.size() < 1) {
        labelPairHr->setText("");
        labelPairHr->fadeOut(500);
        return;
    }


    foreach (Sensor sensor, lstSensor) {
        /// HR
        if (sensor.getDeviceType() == Sensor::SENSOR_HR) {
            sensorHr = sensor;
            usingHr = true;
        }
        /// SC
        else if (sensor.getDeviceType() == Sensor::SENSOR_SPEED_CADENCE) {
            sensorSpeedCadence = sensor;
            usingSpeedCadence = true;
        }
        /// Cadence
        else if (sensor.getDeviceType() == Sensor::SENSOR_CADENCE) {
            sensorCadence = sensor;
            usingCadence = true;
        }
        /// Speed
        else if (sensor.getDeviceType() == Sensor::SENSOR_SPEED) {
            sensorSpeed = sensor;
            usingSpeed = true;
        }
        /// Power
        else if (sensor.getDeviceType() == Sensor::SENSOR_POWER) {
            sensorPower = sensor;
            usingPower = true;
        }
        /// FE-C
        else if (sensor.getDeviceType() == Sensor::SENSOR_FEC) {
            sensorFEC = sensor;
            usingFEC = true;
            idFecMainUser = sensor.getAntId();
        }
        /// Oxygen
        else if (sensor.getDeviceType() == Sensor::SENSOR_OXYGEN) {
            sensorOxygen = sensor;
            usingOxygen = true;
        }
    }


    /// If not using specific sensor, dont check for pairing completion
    if (!usingHr)
        hrPairingDone = true;
    if (!usingSpeedCadence)
        scPairingDone = true;
    if (!usingCadence)
        cadencePairingDone = true;
    if (!usingSpeed)
        speedPairingDone = true;
    if (!usingPower)
        powerPairingDone = true;
    if (!usingFEC)
        fecPairingDone = true;
    if (!usingOxygen)
        oxygenPairingDone = true;



    //// --------------- Show pairing window --------
    QString pairingWithMsg = tr(" Pairing with %1 sensor (%2, %3)...");

    labelPairHr->setText(""); //Erase previous msg

    if (usingHr) {
        QString pairingMsgHr = pairingWithMsg.arg(Sensor::getName(Sensor::SENSOR_HR), QString::number(sensorHr.getAntId()) , sensorHr.getName());
        labelPairHr->setText(pairingMsgHr);
        labelPairHr->setVisible(true);
    }
    if (usingSpeedCadence) {
        QString pairingMsgSC = pairingWithMsg.arg(Sensor::getName(Sensor::SENSOR_SPEED_CADENCE), QString::number(sensorSpeedCadence.getAntId()) , sensorSpeedCadence.getName());
        labelSpeedCadence->setText(pairingMsgSC);
        labelSpeedCadence->setVisible(true);
    }
    if (usingCadence) {
        QString pairingMsgCadence = pairingWithMsg.arg(Sensor::getName(Sensor::SENSOR_CADENCE), QString::number(sensorCadence.getAntId()) , sensorCadence.getName());
        labelCadence->setText(pairingMsgCadence);
        labelCadence->setVisible(true);
    }
    if (usingSpeed) {
        QString pairingMsgSpeed = pairingWithMsg.arg(Sensor::getName(Sensor::SENSOR_SPEED), QString::number(sensorSpeed.getAntId()) , sensorSpeed.getName());
        labelSpeed->setText(pairingMsgSpeed);
        labelSpeed->setVisible(true);
    }
    if (usingFEC) {
        QString pairingMsgFEC = pairingWithMsg.arg(Sensor::getName(Sensor::SENSOR_FEC), QString::number(sensorFEC.getAntId()) , sensorFEC.getName());
        labelFEC->setText(pairingMsgFEC);
        labelFEC->setVisible(true);
    }
    if (usingOxygen) {
        QString pairingMsgOxy = pairingWithMsg.arg(Sensor::getName(Sensor::SENSOR_OXYGEN), QString::number(sensorOxygen.getAntId()) , sensorOxygen.getName());
        labelOxygen->setText(pairingMsgOxy);
        labelOxygen->setVisible(true);
    }
    if (usingPower) {
        QString pairingMsgPower = pairingWithMsg.arg(Sensor::getName(Sensor::SENSOR_POWER), QString::number(sensorPower.getAntId()) , sensorPower.getName());
        labelPower->setText(pairingMsgPower);
        labelPower->setVisible(true);
    }
    else if (account->powerCurve.getId() > 0 && (usingSpeedCadence || usingSpeed) ) {
        labelPower->setText(tr(" Using Trainer Power Curve:") + account->powerCurve.getFullName());
        labelPower->setStyleSheet("background-color : rgba(1,1,1,220); color : green;");
        labelPower->setVisible(true);
    }


    labelPairHr->fadeIn(500);
    labelSpeedCadence->fadeIn(500);
    labelCadence->fadeIn(500);
    labelSpeed->fadeIn(500);
    labelFEC->fadeIn(500);
    labelOxygen->fadeIn(500);
    labelPower->fadeIn(500);
    //////////////////////////


    qDebug() << "Ok start sensors now !";
    sendSoloData(account->powerCurve, account->wheel_circ, lstSensor, account->use_pm_for_cadence, account->use_pm_for_speed);



    qDebug() << "slotGetSensorListFinished";
}
//---------------------------------------------------------------------------------
void FloatingWorkout::checkPairingCompleted() {

    if (hrPairingDone && scPairingDone && cadencePairingDone && speedPairingDone && powerPairingDone && oxygenPairingDone && fecPairingDone)
    {

        labelPairHr->fadeOutAfterPause(2000, 6000);
        labelSpeedCadence->fadeOutAfterPause(2000, 6000);
        labelCadence->fadeOutAfterPause(2000, 6000);
        labelSpeed->fadeOutAfterPause(2000, 6000);
        labelPower->fadeOutAfterPause(2000, 6000);
        labelFEC->fadeOutAfterPause(2000, 6000);
        labelOxygen->fadeOutAfterPause(2000, 6000);

    }
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::closeWindow() {

    sureYouWantToQuit();
}

void FloatingWorkout::minimizeWindow() {

    this->showMinimized();
}


//////////////////////////////////////////////////////
void FloatingWorkout::expandWindow() {


    if (this->isFullScreen()) {
        qDebug() << "going call showNormalWin ";
        showNormalWin();
    }
    else {
        qDebug() << "going call showFullScreenWin ";
        showFullScreenWin();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::reject() {

    sureYouWantToQuit();
}


//------------------------------------------------------------------------------------------------------
void FloatingWorkout::sureYouWantToQuit() {

    if (!isWorkoutPaused) {
        start_or_pause_workout();
    }

    if (isWorkoutStarted && !isWorkoutOver) {

        isAskingUserQuestion = true;
        QMessageBox msgBox(this);
        msgBox.setStyleSheet("QLabel {color: black;}");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("Workout is not completed."));
        msgBox.setInformativeText(tr("Save your progress?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);

        int reply = msgBox.exec();
        if (reply == QMessageBox::Yes) {

            // Save FIT FILE
            if (Util::getSystemPathHistory() != "invalid_writable_path") {
                int intervalPausedTime_msec = totalTimePausedWorkout_msec - lastIntervalTotalTimePausedWorkout_msec;
                changeIntervalsDataWorkout(lastIntervalEndTime_msec, timeElapsed_sec, intervalPausedTime_msec, true, false);
                closeFitFiles(timeElapsed_sec);
            }
            QDialog::done(1);
        }
        else if (reply == QMessageBox::No) {
            closeAndDeleteFitFile();
            QDialog::done(1);
        }
        else {
            qDebug() << "Cancel was clicked";
        }
        isAskingUserQuestion = false;
    }
    /// Not started or workout completed, no warning to show
    else {
        QDialog::done(1);
    }

}


//------------------------------------------------------------
void FloatingWorkout::ignoreClickPlot() {
    bignoreClickPlot = false;
    timerIgnoreClickPlot->stop();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::activateSoundBool() {
    soundsActive = true;
    timerCheckToActivateSound->stop();
}
void FloatingWorkout::activateSoundPowerTooLow() {
    soundPowerTooLowActive = true;
    timerCheckReactivateSoundPowerTooLow->stop();
}
void FloatingWorkout::activateSoundPowerTooHigh() {
    soundPowerTooHighActive = true;
    timerCheckReactivateSoundPowerTooHigh->stop();
}
void FloatingWorkout::activateSoundCadenceTooLow() {
    soundCadenceTooLowActive = true;
    timerCheckReactivateSoundCadenceTooLow->stop();
}
void FloatingWorkout::activateSoundCadenceTooHigh() {
    soundCadenceTooHighActive = true;
    timerCheckReactivateSoundCadenceTooHigh->stop();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::checkFitFileCreated() {

    qDebug() << "****checkFitFileCreated";
    QString fitFilename = arrDataWorkout[0]->getFitFilename();
    qDebug() << "FIT File name is" << fitFilename << "name:"<< workout.getName() << "desc" << workout.getDescription();
    if (fitFilename != "") {
        emit fitFileReady(fitFilename, workout.getName(), workout.getDescription());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::closeFitFiles(double timeElapSec) {


    if (account->enable_studio_mode) {
        for (int i=0; i<account->nb_user_studio; i++) {
            arrDataWorkout[i]->writeEndFile(timeElapSec);
            arrDataWorkout[i]->closeFitFile();
        }
    }
    else {
        arrDataWorkout[0]->writeEndFile(timeElapSec);
        arrDataWorkout[0]->closeFitFile();
        checkFitFileCreated();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::closeAndDeleteFitFile() {

    if (account->enable_studio_mode) {
        for (int i=0; i<account->nb_user_studio; i++) {
            arrDataWorkout[i]->closeAndDeleteFitFile();
        }
    }
    else {
        arrDataWorkout[0]->closeAndDeleteFitFile();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::changeIntervalsDataWorkout(double timeStarted, double timeNow, int timePaused_msec, bool workoutOver, bool testInterval) {

    qDebug () << "changeIntervalsDataWorkout! - timeStarted" << timeStarted << "timeNow" << timeNow << "timePaused_msec" << timePaused_msec << "workoutOver" << workoutOver << "testInterval" << testInterval;

    if (account->enable_studio_mode) {
        for (int i=0; i<account->nb_user_studio; i++) {
            //            qDebug() << "change interval for this user" << i;
            arrDataWorkout[i]->changeInterval(timeStarted, timeNow, timePaused_msec, workoutOver, testInterval);
        }
    }
    else {
        qDebug() << "OK change interval, timePaused_msec: " << timePaused_msec;
        arrDataWorkout[0]->changeInterval(timeStarted, timeNow, timePaused_msec, workoutOver, testInterval);
    }

    lastIntervalEndTime_msec = static_cast<int>(timeElapsed_sec);
    lastIntervalTotalTimePausedWorkout_msec = totalTimePausedWorkout_msec;
    emit sendOxygenCommand(sensorOxygen.getAntId(), Oxygen_Controller::LAP);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::initDataWorkout() {

    if (account->enable_studio_mode) {
        for (int i=0; i<account->nb_user_studio; i++) {
            qDebug() << "Create DataWorkout for this user" << i;
            arrDataWorkout[i] = new DataWorkout(this->workout, account->getFTP(), this);
        }
    }
    else {
        arrDataWorkout[0] = new DataWorkout(this->workout, account->getFTP(), this);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::showTestResult() {




    // ------------- FTP TEST -----------------
    if (workout.getWorkoutNameEnum() == Workout::FTP_TEST || workout.getWorkoutNameEnum() == Workout::FTP8min_TEST) {
        int newFTP = arrDataWorkout[0]->getFTP();
        int newLTHR = arrDataWorkout[0]->getLTHR();
        mainPlot->setAlertMessage(true, false, workout.getName() + tr(" Result")
                                  + "<div style='color:white;height:7px;'>------------------------------------</div><br/> "
                                  + tr("FTP: ") + QString::number(newFTP) + tr(" watts") + tr(" (Previous: ") +  QString::number(account->getFTP()) + tr(" watts)") + "<br/>"
                                  + tr("LTHR: ")  + QString::number(newLTHR) + tr(" bpm") + tr(" (Previous: ") +  QString::number(account->getLTHR()) + tr(" bpm)"), 500);
        if (newFTP > 0) { account->setFTP(newFTP); }
        if (newLTHR > 0) { account->setLTHR(newLTHR); }
        emit ftp_lthr_changed();
    }

    // ------------- CP TEST -----------------
    else if (workout.getWorkoutNameEnum() ==  Workout::CP5min_TEST || workout.getWorkoutNameEnum() ==  Workout::CP20min_TEST) {
        int criticalPower = arrDataWorkout[0]->getCP();
        mainPlot->setAlertMessage(true, false, workout.getName() + tr(" Result")
                                  + "<div style='color:white;height:7px;'>------------------</div><br/> "
                                  + QString::number(criticalPower) + tr(" watts"), 500);
    }
    else {
        mainPlot->setAlertMessage(true, false, workout.getName() + tr(" completed!")
                                  + "<div style='color:white;height:7px;'>------------------------------------</div><br/> "
                                  + tr("Nice work! "), 500);
    }




}


/////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::sendUserInfoToClock() {


    if (account->enable_studio_mode) {
        for (int i=0; i<account->nb_user_studio; i++) {
            //            UserStudio myUserStudio = vecUserStudio.at(i);
            //            double cda = 30;
            //            double weight = 80;
            emit sendUserInfo(i+1, account->userCda, account->bike_weight_kg + account->getWeightKg(), account->nb_user_studio);
        }
    }
    else {
        emit sendUserInfo(1, account->userCda, account->bike_weight_kg + account->getWeightKg(), 1);
    }
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::connectHubs() {

    for (int i=0; i<vecHub.size(); i++) {

        if (vecStickIdUsed.contains(i)) {

            Hub *hub = vecHub.at(i);

            connect(hub, SIGNAL(signal_hr(int, int)), this, SLOT(HrDataReceived(int, int)) );
            connect(hub, SIGNAL(signal_cadence(int, int)), this, SLOT(CadenceDataReceived(int, int)) );
            connect(hub, SIGNAL(signal_power(int, int)), this, SLOT(PowerDataReceived(int, int)) );
            connect(hub, SIGNAL(pedalMetricChanged(int, double,double,double,double,double)), this, SLOT(pedalMetricReceived(int, double,double,double,double,double)));
            connect(hub, SIGNAL(signal_batteryLow(QString,int,int)), this, SLOT(batteryStatusReceived(QString,int,int)) );

            connect(hub, SIGNAL(askPermissionForSensor(int,int)), this, SLOT(addToControlList(int,int)) );
            connect(this, SIGNAL(permissionGrantedControl(int,int)), hub, SLOT(addToControlListHub(int,int)) );

            // Trainer Control signals
            connect(this, SIGNAL(setLoad(int, double)), hub, SLOT(setLoad(int, double)));
            connect(this, SIGNAL(setSlope(int, double)), hub, SLOT(setSlope(int, double)));
            // Stop decoding when this Window is closed to save cpu
            connect(this, SIGNAL(stopDecodingMsgHub()), hub, SLOT(stopDecodingMsg()) );


            // Enable Lap To be changed by Trainer (FEC) only in Solo Mode, Oxygen only available in Solo mode
            if (!account->enable_studio_mode) {
                connect(hub, SIGNAL(signal_lapChanged(int)), this, SLOT(lapButtonPressed()) );
                connect(this, SIGNAL(sendOxygenCommand(int, Oxygen_Controller::COMMAND)), hub, SLOT(sendOxygenCommand(int, Oxygen_Controller::COMMAND)) );
                connect(this, SIGNAL(sendSoloData(PowerCurve,int, QList<Sensor>, bool, bool)), hub, SLOT(setSoloDataToHub(PowerCurve,int, QList<Sensor>, bool, bool)) );
            }
            // Studio Mode
            else {
                connect(this, SIGNAL(sendDataUserStudio(QVector<UserStudio>)), hub, SLOT(setUserStudioData(QVector<UserStudio>)) );
            }
        }

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::connectDataWorkout() {

    qDebug() <<  "connectDataWorkout";

}

///////////////////////////////////////////////////////////////////////////////////////////////////////
bool FloatingWorkout::eventFilter(QObject *watched, QEvent *event) {

    Q_UNUSED(watched)

//    if(event->type() != QEvent::Paint && event->type() != QEvent::UpdateRequest && event->type() != QEvent::MetaCall && event->type() != QEvent::ChildAdded && event->type() != QEvent::ChildPolished && event->type() != QEvent::PolishRequest && event->type() != QEvent::LayoutRequest){
//        qDebug() << "EventFilter " << watched << "Event:" << event->type();
//    }

    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return ) {
            qDebug() << "ENTER PRESSED- STOP/START WORKOUT!" << watched;
            start_or_pause_workout();
            return true; // mark the event as handled
        }
        else if (keyEvent->key() == Qt::Key_Up) {
            emit increaseDifficulty();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Down) {
            emit decreaseDifficulty();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Backspace) {
            lapButtonPressed();
            return true;
        }
        // --- Calibration
        else if (keyEvent->key() == Qt::Key_F1) {
            qDebug() << "F1";
            if (usingFEC) {
                //startCalibrateFEC();
            }
            else if (usingPower) {
                //startCalibrationPM();
            }
            return true;
        }

        else if (keyEvent->key() == Qt::Key_F2) {
            qDebug() << "F2";
            return true;
        }
        // --- Fullscreen
        else if (keyEvent->key() == Qt::Key_F11) {
            expandWindow();
            return true;
        }

    }

    if (event->type() == QEvent::Enter)
    {
        ui->button_close->show();
    }

    if (event->type() == QEvent::Leave){
        ui->button_close->hide();
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FloatingWorkout::showFullScreenWin() {

    qDebug() << "showFullScreenWin";
    this->setSizeGripEnabled(false);

#ifdef Q_OS_MAC
    this->showMaximized();
#else
    this->showFullScreen();
#endif

    isFullScreenFlag = true;
}

//-----------------------------------------
void FloatingWorkout::showNormalWin() {

    qDebug() << "showNormalWin";


#ifdef Q_OS_MAC
    this->showNormal();
#else
    this->showNormal();
#endif

    this->setSizeGripEnabled(true);
    //    ui->widget_topMenu->setMinExpandExitVisible(false);

    isFullScreenFlag = false;

}


void FloatingWorkout::changeWidget(){
    //if(hrOpacityEffect->opacity() == 0.0){
    if(ui->wid_1_minimalistHr->isHidden()){
        toggleWidget(ui->wid_3_minimalistCadence, hrCadAnimation, cadOpacityEffect, false);
    }else{
        toggleWidget(ui->wid_1_minimalistHr, hrCadAnimation, hrOpacityEffect, false);
    }

    if(ui->scrollArea->horizontalScrollBar()->value() == 0){
        plotAnimation->setStartValue(0);
        plotAnimation->setEndValue(ui->scrollArea->horizontalScrollBar()->maximum());
        plotAnimation->start();
    }else{
        plotAnimation->setStartValue(ui->scrollArea->horizontalScrollBar()->maximum());
        plotAnimation->setEndValue(0);
        plotAnimation->start();
    }
}

void FloatingWorkout::toggleWidget(QWidget *widget, QPropertyAnimation *animation, QGraphicsOpacityEffect *effect, bool isShow){
    int startValue = 1;
    int endValue = 0;
    if(isShow){
        widget->show();
        startValue = 0;
        endValue = 1;
    }
    animation->setTargetObject(effect);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->start();
}

void FloatingWorkout::hrCadAnimationFinished(){
    if(hrCadAnimation->targetObject()->objectName() == "hrOpacityEffect"){
        if(hrOpacityEffect->opacity() == 0.0){
            ui->wid_1_minimalistHr->hide();
            toggleWidget(ui->wid_3_minimalistCadence, hrCadAnimation, cadOpacityEffect, true);
        }
    }else{
        if(cadOpacityEffect->opacity() == 0.0){
            ui->wid_3_minimalistCadence->hide();
            toggleWidget(ui->wid_1_minimalistHr, hrCadAnimation, hrOpacityEffect, true);
        }
    }
}

void FloatingWorkout::fakeData() {
//    int value = (qrand() % ((150 + 1) - 130) + 130);
//    HrDataReceived(1, value);
//    value = (qrand() % ((75 + 1) - 60) + 60);
//    CadenceDataReceived(1, value);

//    qint64 current = QDateTime::currentSecsSinceEpoch();
//    if(current > lastFakeChangeTime + 1){
//        originFakePower += 20;
//        lastFakeChangeTime = current;
//    }

//    value = qrand() % 10 + originFakePower;
//    PowerDataReceived(1, value);

}

void FloatingWorkout::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        isMouseDown = true;
        m_startPoint = event->globalPos();
        m_windowPoint = this->frameGeometry().topLeft();
    }
}

void FloatingWorkout::mouseMoveEvent(QMouseEvent *event){
    if (event->buttons() & Qt::LeftButton && isMouseDown){
        QPoint relativePos = event->globalPos() - m_startPoint;
        this->move(m_windowPoint + relativePos );
    }
}

void FloatingWorkout::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        isMouseDown = false;
        this->savePosSettings();
    }
}

void FloatingWorkout::on_button_close_clicked()
{
    sureYouWantToQuit();
}

void FloatingWorkout::targetDiffTypeChanged(int diff){

    if(diff * this->diffLevel <= 0){
        if(colorRGBAnimation->state() == QVariantAnimation::Running){
            colorRGBAnimation->stop();
        }
        QVariantAnimation *redAnimation = dynamic_cast<QVariantAnimation *>(colorRGBAnimation->animationAt(0));
        QVariantAnimation *greenAnimation = dynamic_cast<QVariantAnimation *>(colorRGBAnimation->animationAt(1));
        QVariantAnimation *blueAnimation = dynamic_cast<QVariantAnimation *>(colorRGBAnimation->animationAt(2));

        redAnimation->setStartValue(this->currentR);
        greenAnimation->setStartValue(this->currentG);
        blueAnimation->setStartValue(this->currentB);

        if(diff<0){
            redAnimation->setEndValue(0);
            greenAnimation->setEndValue(0);
            blueAnimation->setEndValue(255);
        }else if(diff == 0){
            redAnimation->setEndValue(this->inrangeWhite);
            greenAnimation->setEndValue(this->inrangeWhite);
            blueAnimation->setEndValue(this->inrangeWhite);
        }else{
            redAnimation->setEndValue(255);
            greenAnimation->setEndValue(0);
            blueAnimation->setEndValue(0);
        }
        colorRGBAnimation->start();
    }

    if(diff != 0){
        this->colorAlphaAnimation->setDuration(4000 / abs(diff));
        this->colorAlphaAnimation->setKeyValueAt(0.5, 50+30*abs(diff));
        if(this->colorAlphaAnimation->state() == QVariantAnimation::Stopped){
            this->colorAlphaAnimation->start();
        }
    }

    this->diffLevel = diff;
}

void FloatingWorkout::backgroundColorChanged(QVariant value){
    QVariantAnimation *redAnimation = dynamic_cast<QVariantAnimation *>(colorRGBAnimation->animationAt(0));
    QVariantAnimation *greenAnimation = dynamic_cast<QVariantAnimation *>(colorRGBAnimation->animationAt(1));
    QVariantAnimation *blueAnimation = dynamic_cast<QVariantAnimation *>(colorRGBAnimation->animationAt(2));

    this->currentR = redAnimation->currentValue().toInt();
    this->currentG = greenAnimation->currentValue().toInt();
    this->currentB = blueAnimation->currentValue().toInt();
    this->currentAlpha = this->colorAlphaAnimation->currentValue().toInt();
    QColor *color = new QColor(this->currentR, this->currentG, this->currentB, this->currentAlpha);

    this->setWidgetBackgroundColor(color);
}

void FloatingWorkout::alphaAnimationFinished(){
    if(this->diffLevel != 0){
        this->colorAlphaAnimation->start();
    }
}

void FloatingWorkout::loadPosFromSettings(){
    QSettings settings;

    settings.beginGroup("FloatingDialog");
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();
}

void FloatingWorkout::savePosSettings(){
    QSettings settings;

    settings.beginGroup("FloatingDialog");
    settings.setValue("pos", pos());
    settings.endGroup();
}
