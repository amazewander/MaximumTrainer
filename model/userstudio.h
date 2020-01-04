#ifndef USERSTUDIO_H
#define USERSTUDIO_H

#include <QtCore>
#include "powercurve.h"


class UserStudio
{


public:
    UserStudio() {}
    UserStudio(QString displayName, int FTP, int LTHR, int hrID, int powerID, int cadenceID, int speedID, int fecID,
               int wheelCircMM, bool usingPowerCurve, int companyID, int brandID);




    //getters
    QString getDisplayName() const {
        return this->displayName;
    }
    int getFTP() const {
        return this->FTP;
    }
    int getLTHR() const {
        return this->LTHR;
    }

    int getHrID() const {
        return this->hrID;
    }
    int getPowerID() const {
        return this->powerID;
    }
    int getCadenceID() const {
        return this->cadenceID;
    }
    int getSpeedID() const {
        return this->speedID;
    }
    int getFecID() const {
        return this->fecID;
    }

    int getWheelCircMM() const {
        return this->wheelCircMM;
    }
    bool getUsingPowerCurve() const {
        return this->usingPowerCurve;
    }
    int getCompanyID() const {
        return this->companyID;
    }
    int getBrandID() const {
        return this->brandID;
    }
    PowerCurve getPowerCurve() const {
        return this->powerCurve;
    }


    //QString displayName;  = 0
    //int FTP;              = 1
    //int LTHR;             = 2
    //int hrID;             = 3
    //int power             = 4
    //int cadenceID;        = 5
    //int speedID;          = 6
    //int fecID;            = 7
    //int wheelCircMM;      = 8

    //Setters
    void setDisplayName(QString displayName) {
        this->displayName = displayName;
    }
    void setFTP(int ftp) {
        this->FTP = ftp;
    }
    void setLTHR(int lthr) {
        this->LTHR = lthr;
    }
    void setHrID(int hrID) {
        this->hrID = hrID;
    }
    void setPowerID(int powerID) {
        this->powerID = powerID;
    }
    void setCadenceID(int cadenceID) {
        this->cadenceID = cadenceID;
    }
    void setSpeedID(int speedID) {
        this->speedID = speedID;
    }
    void setFecID(int fecID) {
        this->fecID = fecID;
    }
    void setWheelCircMM(int wheelCircMM) {
        this->wheelCircMM = wheelCircMM;
    }

    void setUsingPowerCurve(bool val) {
        this->usingPowerCurve = val;
    }
    void setCompanyID(int id) {
        this->companyID = id;
    }
    void setBrandID(int id) {
        this->brandID = id;
    }

    void setPowerCurve(PowerCurve curve) {
        this->powerCurve = curve;
    }


private :

    QString displayName;
    int FTP;
    int LTHR;

    int hrID;
    int powerID;
    int cadenceID;
    int speedID;
    int fecID;

    int wheelCircMM;

    bool usingPowerCurve;
    int companyID;
    int brandID;

    PowerCurve powerCurve;


};
Q_DECLARE_METATYPE(UserStudio)

#endif // USERSTUDIO_H
