#include "fitactivitycreator.h"


#include "fit_decode.hpp"
#include "fit_mesg_broadcaster.hpp"
#include "fit_file_id_mesg.hpp"


//#include <cstdlib>


#include <QFile>

#include "account.h"
#include "util.h"



FitActivityCreator::FitActivityCreator() {


    fileIsComplete = false;
    fileIsOpen = false;
    totalTimePaused = 0.0;
    accumulatedDistance = 0.0;
    timeSecLastWriteRecord = 0;

    convertKphToMs = 0.277778;

    studioMode = false;
}



//---------------------------------------------------------------------------------------------------------------------------------
QString FitActivityCreator::generateFileName(bool createDir, QString username, QString nameWorkout, QDateTime startTimeWorkout) {

    qDebug() << "generateFileName";


    QString nameFile = Util::getSystemPathHistory() + "/";
    if (createDir) {
        nameFile +=  startTimeWorkout.toLocalTime().toString("yyyy-MM-dd(hh-mm-ss)") + "_Studio_" + nameWorkout + "/";
        folderPathStudio = nameFile;
        QDir().mkdir(folderPathStudio);
        studioMode = true;
    }

    nameFile += startTimeWorkout.toLocalTime().toString("yyyy-MM-dd(hh-mm-ss)") + "-MT-" + username + "-" + nameWorkout + ".fit";
    qDebug() << "name of fit file:" << nameFile;


    return nameFile;
}




//------------------------------------------------------------------------------------------------------------
void FitActivityCreator::initialize_FIT_File(bool createDir, QString username, QString nameWorkout, QDateTime startTimeWorkout) {


    qDebug() << "initialize_FIT_File";
    fileName = generateFileName(createDir, username, nameWorkout, startTimeWorkout);



#ifdef Q_OS_MAC
    file.open(fileName.toStdString(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
#endif
#ifdef Q_OS_WIN32
    file.open(fileName.toStdWString(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
#endif




    if (!file.is_open()) {
        qDebug() << "Error opening file" << fileName;
        return;
    }

    encode.Open(file);
    fileIsOpen = true;


    // TimeCreated - seconds since UTC 00:00 Dec 31 1989
    QDateTime now =  QDateTime::currentDateTime().toUTC();
    QDateTime ref = QDateTime(QDate(1989,12,31), QTime(0,0), Qt::UTC);
    secStartedWorkout = ref.secsTo(now);



    // --------------- FileIdMesg ------------------
    fit::FileIdMesg fileIdMesg;
    fileIdMesg.SetTimeCreated(secStartedWorkout);
    fileIdMesg.SetManufacturer(FIT_MANUFACTURER_DEVELOPMENT);
    fileIdMesg.SetType(FIT_FILE_ACTIVITY);
    encode.Write(fileIdMesg);


    fit::FileCreatorMesg fileCreatorMesg;
    fileCreatorMesg.SetSoftwareVersion(303);  //3.03
    encode.Write(fileCreatorMesg);

    // deviceInfo - needed for Garmin Connect upload...
    fit::DeviceInfoMesg deviceInfo;
    deviceInfo.SetTimestamp(secStartedWorkout);
    deviceInfo.SetManufacturer(FIT_MANUFACTURER_DEVELOPMENT);
    encode.Write(deviceInfo);


}




//------------------------------------------------------------------------------------------------------------
void FitActivityCreator::close_FIT_File() {


    qDebug() << "close_FIT_File, fileisOpen?" << fileIsOpen;

    if (!fileIsOpen)
        return;

    qDebug() << "closeFitFile!";

    if (!encode.Close()) {
        qDebug() << "Error closing encode\n";
    }
    file.close();
    qDebug() << "Encoded FIT file test.fit\n";


    fileIsComplete = true;


}


//------------------------------------------------------------------------------------------------------------
void FitActivityCreator::closeAndDeleteFile() {

    if (!fileIsOpen)
        return;

    close_FIT_File();

    if (studioMode) {
        QDir myDir(folderPathStudio);
        myDir.removeRecursively();
    }
    else {
        QFile myfile(fileName);
        myfile.remove();
    }





}

//------------------------------------------------------------------------------------------------------------
void FitActivityCreator::writeEndFile(double timeNow,
                                      double avgSpeedKph, double avgCadence, double avgPower, double avgHr,
                                      double maxSpeedKph, double maxCadence, double maxPower, double maxHr,
                                      double calories, double normalizedPower, double intensityFactor, double trainingStressScore,
                                      double nbLaps ) {


    if (!fileIsOpen)
        return;

    qDebug() <<  "Fit:writeEndFile" << timeNow;


    qDebug() << "SEC SARTED WORKOUT:" << secStartedWorkout;



    // --------------- SessionMesg ------------------
    fit::SessionMesg sessionMesg;
    sessionMesg.SetTimestamp(secStartedWorkout+timeNow);
    sessionMesg.SetStartTime(secStartedWorkout);
    sessionMesg.SetTotalElapsedTime(timeNow + totalTimePaused); //Total number of msec since timer started (includes pauses)
    sessionMesg.SetTotalTimerTime(timeNow); //Timer Time (excludes pauses)
    //    sessionMesg.SetTotalMovingTime(timeNow);
    sessionMesg.SetSport(FIT_SPORT_CYCLING);
    sessionMesg.SetSubSport(FIT_SUB_SPORT_INDOOR_CYCLING);
    sessionMesg.SetMessageIndex(0);
    sessionMesg.SetFirstLapIndex(0);
    sessionMesg.SetNumLaps(nbLaps);
    sessionMesg.SetTrigger(FIT_SESSION_TRIGGER_ACTIVITY_END);


    if (avgSpeedKph > 0) {
        double timeHrs = timeNow/3600.0;
        double distanceM = avgSpeedKph * timeHrs *1000;
        sessionMesg.SetTotalDistance(distanceM);

        sessionMesg.SetAvgSpeed(avgSpeedKph*convertKphToMs);
        sessionMesg.SetMaxSpeed(maxSpeedKph);
    }
    if (avgCadence > 0) {
        sessionMesg.SetAvgCadence(avgCadence);
        sessionMesg.SetMaxCadence(maxCadence);
    }
    if (avgPower > 0) {
        sessionMesg.SetAvgPower(avgPower);
        sessionMesg.SetMaxPower(maxPower);
    }
    if (avgHr > 0) {
        sessionMesg.SetAvgHeartRate(avgHr);
        sessionMesg.SetMaxHeartRate(maxHr);
    }
    if (normalizedPower > 0) {
        sessionMesg.SetTotalCalories(calories);
        sessionMesg.SetNormalizedPower(normalizedPower);
        sessionMesg.SetIntensityFactor(intensityFactor);
        sessionMesg.SetTrainingStressScore(trainingStressScore);
    }


    encode.Write(sessionMesg);






    // --------------- ActivityMesg ------------------
    fit::ActivityMesg activityMesg;
    activityMesg.SetTimestamp(secStartedWorkout+timeNow);
    activityMesg.SetTotalTimerTime(timeNow);
    activityMesg.SetNumSessions(1);  // Always 1 session (1 workout per file)
    activityMesg.SetType(0);
    activityMesg.SetEvent(FIT_EVENT_ACTIVITY);
    activityMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    encode.Write(activityMesg);





}


//------------------------------------------------------------------------------------------------------------
void FitActivityCreator::writeLapMesg(double timeStarted, double timeNow, int timePaused_msec,
                                      double avgSpeedKph, double avgCadence, double avgPower, double avgHr,
                                      double maxSpeedKph, double maxCadence, double maxPower, double maxHr,
                                      double lapCalories) {

    if (!fileIsOpen)
        return;

    double durationInterval = timeNow - timeStarted;
    double durationPause = timePaused_msec /1000.0;

    totalTimePaused += durationPause;

    qDebug() << "timeStarted" << timeStarted << "timeNow" << timeNow << "Duration:" << durationInterval;
    qDebug() << "WE WERE PAUSED FOR " << durationPause;

    if (!fileIsOpen)
        return;


    fit::LapMesg lapMesg;
    lapMesg.SetTimestamp(secStartedWorkout+timeNow);
    lapMesg.SetStartTime(secStartedWorkout + timeStarted + 1);
    lapMesg.SetTotalElapsedTime(durationInterval + durationPause ); //includes pauses
    lapMesg.SetTotalTimerTime(durationInterval);  //excludes pauses
    //    lapMesg.SetTotalMovingTime(durationInterval);



    if (avgSpeedKph > 0) {
        double timeHrs = durationInterval/3600.0;
        double distanceM = avgSpeedKph * timeHrs *1000;
        lapMesg.SetTotalDistance(distanceM);

        lapMesg.SetAvgSpeed(avgSpeedKph*convertKphToMs);
        lapMesg.SetMaxSpeed(maxSpeedKph);
    }
    if (avgCadence > 0) {
        lapMesg.SetAvgCadence(avgCadence);
        lapMesg.SetMaxCadence(maxCadence);
    }
    if (avgPower > 0) {
        lapMesg.SetAvgPower(avgPower);
        lapMesg.SetMaxPower(maxPower);
    }
    if (avgHr > 0) {
        lapMesg.SetAvgHeartRate(avgHr);
        lapMesg.SetMaxHeartRate(maxHr);
    }

    if (lapCalories > 0) {
        lapMesg.SetTotalCalories(lapCalories);
    }


    lapMesg.SetEvent(FIT_EVENT_LAP);
    lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    lapMesg.SetIntensity(FIT_INTENSITY_ACTIVE);
    lapMesg.SetLapTrigger(FIT_LAP_TRIGGER_TIME);
    lapMesg.SetSport(FIT_SPORT_CYCLING);
    encode.Write(lapMesg);




    //#define FIT_INTENSITY_ACTIVE                                                     ((FIT_INTENSITY)0)
    //#define FIT_INTENSITY_REST                                                       ((FIT_INTENSITY)1)
    //#define FIT_INTENSITY_WARMUP                                                     ((FIT_INTENSITY)2)
    //#define FIT_INTENSITY_COOLDOWN                                                   ((FIT_INTENSITY)3)
    //#define FIT_INTENSITY_COUNT                                                      4
    //    lapMesg.SetIntensity();
}



//----------------------------------------------------------------------------------------------------------------------
void FitActivityCreator::writeRecord(int timeNow,
                                     double averageHr1sec, double averageCadence1sec, double averageSpeed1sec, double averagePower1sec,
                                     double rightPedal, double avgLeftTorqueEff, double avgRightTorqueEff, double avgLeftPedalSmooth, double avgRightPedalSmooth, double avgCombinedPedalSmooth,
                                     double saturatedHemoglobinPercent, double totalHemoglobinConc) {


    if (!fileIsOpen)
        return;

    fit::RecordMesg recordMesg;
    recordMesg.SetTimestamp(secStartedWorkout+timeNow);


    if (averageHr1sec != -1) {
        recordMesg.SetHeartRate((int)(averageHr1sec + 0.5));
    }

    if (averageCadence1sec != -1) {
        recordMesg.SetCadence((int)(averageCadence1sec + 0.5));
    }

    if (averageSpeed1sec != -1) {

        double speedms = averageSpeed1sec*convertKphToMs;
        recordMesg.SetSpeed(speedms);

        // Compute Accumulated Distance
        if (averageSpeed1sec > 0) {
            int timeSec = timeNow - timeSecLastWriteRecord;
            double timeHrs = timeSec/3600.0;
            double distanceM = averageSpeed1sec * timeHrs *1000;
            accumulatedDistance += distanceM;
            recordMesg.SetDistance(accumulatedDistance);
        }
        else {
            recordMesg.SetDistance(accumulatedDistance);
        }
        timeSecLastWriteRecord = timeNow;
    }


    if (averagePower1sec != -1) {
        recordMesg.SetPower((int)(averagePower1sec + 0.5));
    }

    if (rightPedal != -1) {
        recordMesg.SetLeftRightBalance(128 + rightPedal);
    }
    if (avgLeftTorqueEff != -1) {
        recordMesg.SetLeftTorqueEffectiveness(avgLeftTorqueEff);
    }
    if (avgRightTorqueEff != -1) {
        recordMesg.SetRightTorqueEffectiveness(avgRightTorqueEff);
    }

    if (avgLeftPedalSmooth != -1) {
        recordMesg.SetLeftPedalSmoothness(avgLeftPedalSmooth);
    }
    if (avgRightPedalSmooth != -1) {
        recordMesg.SetRightPedalSmoothness(avgRightPedalSmooth);
    }
    if (avgCombinedPedalSmooth != -1) {
        recordMesg.SetCombinedPedalSmoothness(avgCombinedPedalSmooth);
    }

    if (saturatedHemoglobinPercent != -1 && totalHemoglobinConc != -1) {
        recordMesg.SetSaturatedHemoglobinPercent(saturatedHemoglobinPercent);
        recordMesg.SetTotalHemoglobinConc(totalHemoglobinConc);
    }


    encode.Write(recordMesg);
}




//------------------------------------------------------------------------------------------------------------
void FitActivityCreator::build_FIT_file() {

    fit::Encode encode;
    std::fstream file;


    file.open("/Users/tourlou2/test2.fit", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    //file.open("C:/test2.fit", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        qDebug() << "Error opening file test.fit\n";
    }
    encode.Open(file);


    // TimeCreated - seconds since UTC 00:00 Dec 31 1989
    QDateTime now =  QDateTime::currentDateTime().toUTC();
    QDateTime ref = QDateTime(QDate(1989,12,31), QTime(0,0), Qt::UTC);
    qint64 diffSecs = ref.secsTo(now);





    // --------------- FileIdMesg ------------------
    fit::FileIdMesg fileIdMesg;
    fileIdMesg.SetSerialNumber(12345);
    fileIdMesg.SetTimeCreated(diffSecs);
    fileIdMesg.SetManufacturer(FIT_MANUFACTURER_GARMIN);
    fileIdMesg.SetProduct(1561);
    fileIdMesg.SetType(FIT_FILE_ACTIVITY);
    encode.Write(fileIdMesg);




    fit::FileCreatorMesg fileCreatorMesg;
    fileCreatorMesg.SetSoftwareVersion(270);
    fileCreatorMesg.SetHardwareVersion(255);
    encode.Write(fileCreatorMesg);


    // deviceInfo (needed for Garmin Connect)
    fit::DeviceInfoMesg deviceInfo;
    deviceInfo.SetTimestamp(diffSecs);
    deviceInfo.SetSerialNumber(12345);
    deviceInfo.SetCumOperatingTime(4294967295);
    deviceInfo.SetManufacturer(1);
    deviceInfo.SetProduct(1561);
    deviceInfo.SetSoftwareVersion(270);
    deviceInfo.SetDeviceType(1);
    deviceInfo.SetHardwareVersion(255);
    encode.Write(deviceInfo);





    // --------------- RecordMesg ------------------
    for (int i=0; i<300; i++) {
        fit::RecordMesg recordMesg;
        recordMesg.SetTimestamp(diffSecs+i);

        recordMesg.SetSpeed(11);  //m/s
        recordMesg.SetHeartRate(110);
        recordMesg.SetCadence(80);
        recordMesg.SetPower(250);
        encode.Write(recordMesg);
    }


    // --------------- LapMesg ------------------
    fit::LapMesg lapMesg2;
    lapMesg2.SetTimestamp(diffSecs+299);
    lapMesg2.SetStartTime(diffSecs);
    lapMesg2.SetTotalElapsedTime(300);
    lapMesg2.SetTotalTimerTime(300);
    lapMesg2.SetEvent(FIT_EVENT_LAP);
    lapMesg2.SetEventType(FIT_EVENT_TYPE_STOP);
    lapMesg2.SetIntensity(FIT_INTENSITY_WARMUP);
    lapMesg2.SetLapTrigger(FIT_LAP_TRIGGER_TIME);
    lapMesg2.SetSport(FIT_SPORT_CYCLING);
    encode.Write(lapMesg2);


    // --------------- RecordMesg ------------------
    //    add some filler data for 0-5min
    for (int i=300; i<599; i++) {
        fit::RecordMesg recordMesg;
        recordMesg.SetTimestamp(diffSecs+i);

        recordMesg.SetSpeed(10);  //m/s
        recordMesg.SetHeartRate(120);
        recordMesg.SetCadence(90);
        recordMesg.SetPower(300);
        encode.Write(recordMesg);
    }

    // --------------- LapMesg ------------------
    fit::LapMesg lapMesg3;
    lapMesg3.SetTimestamp(diffSecs+600);
    lapMesg3.SetStartTime(diffSecs+300);
    lapMesg3.SetTotalElapsedTime(300.5);
    lapMesg3.SetTotalTimerTime(300.5);
    lapMesg3.SetEvent(FIT_EVENT_LAP);
    lapMesg3.SetEventType(FIT_EVENT_TYPE_STOP);
    lapMesg3.SetIntensity(FIT_INTENSITY_ACTIVE);
    lapMesg3.SetLapTrigger(FIT_LAP_TRIGGER_SESSION_END);
    lapMesg3.SetSport(FIT_SPORT_CYCLING);
    encode.Write(lapMesg3);






    // --------------- SessionMesg ------------------
    fit::SessionMesg sessionMesg;
    sessionMesg.SetTimestamp(diffSecs+600);
    sessionMesg.SetStartTime(diffSecs);
    sessionMesg.SetTotalElapsedTime(600.5); //Total number of msec since timer started (includes pauses) - Todo: calculate paused time
    sessionMesg.SetTotalTimerTime(600.5); //Timer Time (excludes pauses)
    sessionMesg.SetSport(FIT_SPORT_CYCLING);
    sessionMesg.SetSubSport(FIT_SUB_SPORT_INDOOR_CYCLING);
    sessionMesg.SetMessageIndex(0);
    sessionMesg.SetFirstLapIndex(0);
    //    sessionMesg.SetNumLaps(1);
    sessionMesg.SetTrigger(FIT_SESSION_TRIGGER_ACTIVITY_END);
    encode.Write(sessionMesg);



    // --------------- ActivityMesg ------------------
    fit::ActivityMesg activityMesg;
    activityMesg.SetTimestamp(diffSecs+600);
    activityMesg.SetTotalTimerTime(600.5);  //10min
    activityMesg.SetNumSessions(1);  // Always 1 session (1 workout per file)
    activityMesg.SetType(0);
    activityMesg.SetEvent(FIT_EVENT_ACTIVITY);
    activityMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    encode.Write(activityMesg);





    if (!encode.Close()) {
        qDebug() << "Error closing encode.\n";
    }
    file.close();
    qDebug() << "Encoded FIT file test.fit.\n";
}






//------------------------------------------------------------------------------------------------------------
//void FitActivityCreator::decode_FIT_file() {

//    fit::Decode decode;
//    fit::MesgBroadcaster mesgBroadcaster;

//    std::fstream file;



//    file.open("C:/test2.fit", std::ios::in | std::ios::binary);

//    if (!file.is_open())
//    {
//        qDebug() << "Error opening file";
//        return;
//    }

//    if (!decode.CheckIntegrity(file))
//    {
//        qDebug() << "FIT file integrity failed.\n";
//        return;
//    }

//    //    mesgBroadcaster.AddListener((fit::FileIdMesgListener &)listener);
//    //    mesgBroadcaster.AddListener((fit::UserProfileMesgListener &)listener);
//    //    mesgBroadcaster.AddListener((fit::MonitoringMesgListener &)listener);
//    //    mesgBroadcaster.AddListener((fit::DeviceInfoMesgListener &)listener);

//    try
//    {
//        mesgBroadcaster.Run(file);
//    }
//    catch (const fit::RuntimeException& e)
//    {
//        printf("Exception decoding file: %s\n", e.what());
//        return;
//    }

//    qDebug() << "Done decoded FIT file!";

//    return;

//}
