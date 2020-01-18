#include "minimalistwidget.h"
#include "ui_minimalistwidget.h"

#include <QDebug>
#include "util.h"


MinimalistWidget::~MinimalistWidget()
{
    delete ui;
}




MinimalistWidget::MinimalistWidget(QWidget *parent) : QWidget(parent), ui(new Ui::MinimalistWidget)
{
    ui->setupUi(this);

    target = -1;
    setStopped(true);

}



///////////////////////////////////////////////////////////////////////////////////////////////
void MinimalistWidget::setStopped(bool b) {
    isStopped = b;
}


///////////////////////////////////////////////////////////////////////////////////////////////
void MinimalistWidget::setTarget(double percentageTarget, int range) {



    int realTarget = percentageTarget;
    if (type == MinimalistWidget::POWER) {
        realTarget = qRound(percentageTarget * FTP);
    }
    else if (type == MinimalistWidget::HEART_RATE) {
        realTarget = qRound(percentageTarget * LTHR);
    }


    this->target = realTarget;
    this->range = range;



    low = target - range - extraAfterRange;
    high = target + range + extraAfterRange;


    /// Low = 0
    /// High = 1
    /// Trouver inconnu zMin et zMax (borne de la zone)
    double zMin = low + extraAfterRange;
    double zMax = high - extraAfterRange;

    double zMin_01 = (zMin - low) / (double)(high - low);
    double zTarget_01 = (target - low) / (double) (high - low);
    double zMax_01 = (zMax - low) / (double)(high - low);


    //    qDebug() << "Low :" << low << "high :" << high;
    //    qDebug() << "zMin_" << zMin_01 << "ZMax_" << zMax_01;
}




///////////////////////////////////////////////////////////////////////////////////////////////
void MinimalistWidget::setValue(int value) {

    if (target < 0) {
        //ui->frame->setStyleSheet("QFrame#frame { background-color :"+ Util::getColor(Util::DONE).name() + "; }");
        ui->label_value->setText(QString::number(value));
        return;
    }


    if(value < 0) {
        ui->label_value->setText("-");
        return;
    }


    /// Convert value to 0:1 format
    double value_01 = (value - low) / (double)(high - low);
    /// Convert value to 0 - 1000 format
    double value_0mille = value_01 * 1000;
    int value_0mille_i = (int) value_0mille;


    //    qDebug() << "******OK VALUE 0_1 is" << value_01;
    //    qDebug() << "VALUE 0MILLE is" << value_0mille << "Int:" << value_0mille_i;


    ui->label_value->setText(QString::number(value));


    /// Check Value is outside of zone, change color of QLabel
    int diff = value - target;
    int diffAbs = qAbs(diff);


    //    qDebug() << "is stopped?" << isStopped;
    //    qDebug() << "zone Target?" << target;
    //    qDebug() << "diff is" << diff << "zoneRange is " << range;

//    if (isStopped || diffAbs <= range)
//    {
//        ui->frame->setStyleSheet("QFrame#frame { background-color :"+ Util::getColor(Util::DONE).name() + "; }");
//    }
//    else
//    {
//        /// Too Strong
//        if (diff > range) {
//            ui->frame->setStyleSheet("QFrame#frame { background-color :"+ Util::getColor(Util::TOO_HIGH).name() + "; }");
//            //            qDebug() << "SHOULD DRAW RED COLOR BG**************";

//        }
//        /// Too Soft
//        else {
//            ui->frame->setStyleSheet("QFrame#frame { background-color :"+ Util::getColor(Util::TOO_LOW).name() + "; }");
//            //            qDebug() << "SHOULD DRAW BLUE COLOR BG**************";
//        }
//    }




}



//////////////////////////////////////////////////////////////
void MinimalistWidget::setUserData(double FTP, double LTHR) {
    this->FTP = FTP;
    this->LTHR = LTHR;
}




///////////////////////////////////////////////////////////////////////////////////////////////
void MinimalistWidget::setTypeWidget(TypeMinimalist type) {


    this->type = type;

    if (type == MinimalistWidget::HEART_RATE) {
        ui->label_image->setStyleSheet("image: url(:/image/icon/heart2)");
        ui->label_image->setToolTip(tr("Heart Rate - bpm"));
        colorSquare = Util::getColor(Util::SQUARE_HEARTRATE);
        //        ui->frame->setStyleSheet("QSlider::handle:horizontal { background:"+ Util::getColor(Util::LINE_HEARTRATE).name() + "; }");
        extraAfterRange = 10;
    }
    else if (type == MinimalistWidget::SPEED) {
        ui->label_image->setStyleSheet("image: url(:/image/icon/speed)");
        ui->label_image->setToolTip(tr("Speed - km/h"));
        //        colorSquare = Util::getColor(Util::SQUARE_POWER);
        //                 ui->horizontalLayout->setStyleSheet("QSlider::handle:horizontal { background:"+ Util::getColor(Util::LINE_CADENCE).name() + "; }");
        //        extraAfterRange = 5;
    }
    else if (type == MinimalistWidget::CADENCE) {
        ui->label_image->setStyleSheet("image: url(:/image/icon/crank2)");
        ui->label_image->setToolTip(tr("Cadence - rpm"));
        colorSquare = Util::getColor(Util::SQUARE_CADENCE);
        //        ui->horizontalSlider->setStyleSheet("QSlider::handle:horizontal { background:"+ Util::getColor(Util::TOO_HIGH).name() + "; }");
        extraAfterRange = 5;
    }
    else if (type == MinimalistWidget::POWER) {
        ui->label_image->setStyleSheet("image: url(:/image/icon/power2)");
        ui->label_image->setToolTip(tr("Power - watts"));
        colorSquare = Util::getColor(Util::SQUARE_POWER);
        //        ui->frame->setStyleSheet("QSlider::handle:horizontal { background:"+ Util::getColor(Util::LINE_POWER).name() + "; }");
        extraAfterRange = 15;
    }
}


void MinimalistWidget::setBackgroundColor(QColor *color){
    ui->frame->setStyleSheet("QFrame#frame { background-color :"+ color->name(QColor::HexArgb) + "; }");
}
