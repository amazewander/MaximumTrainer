#ifndef FITACTIVITYCREATOR_H
#define FITACTIVITYCREATOR_H


#include <QString>
#include <QDateTime>

#include "fit_encode.hpp"
#include <fstream>

#include <iostream>


class FitActivityCreator
{

public:
    FitActivityCreator();


    //------------------------------
    QString getFilename() {
        return fileName;
    }
    bool getFileIsComplete() {
        return fileIsComplete;
    }
    //------------------------------



//    void decode_FIT_file();
    void build_FIT_file();


    //-------------
    void initialize_FIT_File(bool createDir, QString username, QString name, QDateTime startTimeWorkout);
    void close_FIT_File();
    void writeEndFile(double timeNow,
                      double avgIntervalSpeedKph, double avgCadence, double avgPower, double avgHr,
                      double maxSpeedKph, double maxCadence, double maxPower, double maxHr,
                      double calories, double normalizedPower, double intensityFactor, double trainingStressScore,
                      double nbLaps);

    void writeLapMesg(double timeStarted, double timeNow, int timePaused_msec,
                      double avgIntervalSpeedKph, double avgCadence, double avgPower, double avgHr,
                      double maxSpeedKph, double maxCadence, double maxPower, double maxHr,
                      double lapCalories);
    void writeRecord(int timeNow,
                     double averageHr1sec, double averageCadence1sec, double averageSpeed1sec, double averagePower1sec,
                     double rightPedal, double avgLeftTorqueEff, double avgRightTorqueEff, double avgLeftPedalSmooth, double avgRightPedalSmooth, double avgCombinedPedalSmooth,
                     double saturatedHemoglobinPercent, double totalHemoglobinConc);


    void closeAndDeleteFile();




private :
    QString generateFileName(bool createDir, QString username, QString name, QDateTime startTimeWorkout);



private :
    QString fileName;
    bool fileIsComplete;
    bool fileIsOpen;

    QString folderPathStudio;
    bool studioMode;

    fit::Encode encode;
    std::fstream file;

    int timeSecLastWriteRecord;
    double accumulatedDistance;
    double totalTimePaused;
    qint64 secStartedWorkout;

    double convertKphToMs;
};

#endif // FITACTIVITYCREATOR_H
