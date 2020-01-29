#include "zoneobject.h"
#include <QDebug>
#include "myconstants.h"


ZoneObject::ZoneObject(QObject *parent) :
    QObject(parent)
{
    this->account = qApp->property("Account").value<Account*>();
}


///---------------------------------------------------------------------------
void ZoneObject::updateFTP(int value) {

    qDebug() << "UPDATE FTP ZONEOBJECT";

    account->setFTP(value);
    emit signal_updateFTP();
}


///---------------------------------------------------------------------------
void ZoneObject::updateLTHR(int value) {

    qDebug() << "update LTHR ZONEOBJECT";

    account->setLTHR(value);
    emit signal_updateLTHR();
}

///---------------------------------------------------------------------------
void ZoneObject::updateUserWeight(double weight, bool isKg) {

    qDebug() << "ZONEOBJECT "<< weight << " in KG?" << isKg;

    double weightKg = weight;
    if (!isKg) {
        weightKg = weight/constants::GLOBAL_CONST_CONVERT_KG_TO_LBS;
    }

    account->setWeightInKg(isKg);
    account->setWeightKg(weightKg);
    //account->powerCurve.setRiderWeightKg(weightKg);
}

///---------------------------------------------------------------------------
 void ZoneObject::updateUserHeight(int height) {

      qDebug() << "updateUserHeight" << height;
     account->setHeightCm(height);


 }
 ///---------------------------------------------------------------------------
  void ZoneObject::updateCDA(double cda) {

      qDebug() << "updateCDA" << cda;

      account->setUserCda(cda);
  }


  ///---------------------------------------------------------------------------
 void ZoneObject::updateWheelSize(int size_mm) {

     qDebug() << "updateWheelSize" << size_mm;
     account->setWheelCirc(size_mm);
 }

 ///---------------------------------------------------------------------------
 void ZoneObject::updateBikeWeight(double weight_kg) {

     qDebug() << "updateBikeWeight" << weight_kg;
     account->setBikeWeightKg(weight_kg);
 }

 ///---------------------------------------------------------------------------
 void ZoneObject::updateBiketype(int bikeType) {

      qDebug() << "updateBiketype" << bikeType;
     account->setBikeType(bikeType);
 }

 ////////////////////////////////////////////////////////////////////////////////////
 void ZoneObject::updateDisplayName(int playerId, QString name) {

     qDebug() << "playerID" << playerId << "name" << name;
     account->setDisplayName(name);


 }
