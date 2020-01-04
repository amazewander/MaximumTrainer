#include "powereditor.h"
#include "ui_powereditor.h"

#include <QDebug>
#include <QPainter>

PowerEditor::~PowerEditor()
{
    delete ui;
}


PowerEditor::PowerEditor(QWidget *parent) : QWidget(parent),ui(new Ui::PowerEditor)
{

    ui->setupUi(this);


#ifdef Q_OS_WIN32
    ui->comboBox_power->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->comboBox_FTPorWatts->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
#endif
#ifdef Q_OS_MAC
    ui->comboBox_power->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ui->comboBox_FTPorWatts->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
#endif

    this->account = qApp->property("Account").value<Account*>();
    editingInWatts = false;


    setAutoFillBackground(true);
    setAttribute(Qt::WA_NoMousePropagation);
    setWindowFlags(Qt::WindowStaysOnTopHint);







    ui->gridLayout->setRowMinimumHeight(0,45);
    ui->gridLayout->setRowMinimumHeight(1,45);

}

//////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::setInterval(const Interval &interval) {
    this->myInterval = interval;

    ui->comboBox_power->setCurrentIndex(myInterval.getPowerStepType());
    on_comboBox_power_currentIndexChanged(ui->comboBox_power->currentIndex());

    ui->spinBox_ftpStart->setValue(myInterval.getFTP_start()*100);
    ui->spinBox_ftpEnd->setValue(myInterval.getFTP_end()*100);
    ui->spinBox_rangePower->setValue(myInterval.getFTP_range());

    ui->checkBox_testInterval->setChecked(myInterval.isTestInterval());


    if (myInterval.getRightPowerTarget() != -1) {
        ui->spinBox_leftBalance->setValue(100-myInterval.getRightPowerTarget());
        ui->spinBox_rightBalance->setValue(myInterval.getRightPowerTarget());
        ui->checkBox_balance->setChecked(true);
        on_checkBox_balance_clicked(true);
    }
    else {
        ui->checkBox_balance->setChecked(false);
        on_checkBox_balance_clicked(false);
    }



}




//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_comboBox_power_currentIndexChanged(int index)
{
    Interval::StepType typeStep = static_cast<Interval::StepType>(index);
    myInterval.setPowerStepType(typeStep);


    /// change interface (hide/show widgets)
    //////////////////////////////////////////////////////////////////////////////////////////

    if (index == 0) { ///None
        ui->spinBox_ftpStart->setVisible(false);
        ui->label_toFtpTxt->setVisible(false);
        ui->spinBox_ftpEnd->setVisible(false);
        ui->comboBox_FTPorWatts->setVisible(false);
        ui->label_acceptedPower->setVisible(false);
        ui->spinBox_rangePower->setVisible(false);
        ui->label_accptedWattsTxt->setVisible(false);

    }
    else if (index == 1) { ///FLAT
        ui->spinBox_ftpEnd->setVisible(false);
        ui->label_toFtpTxt->setVisible(false);

        ui->spinBox_ftpStart->setVisible(true);
        ui->comboBox_FTPorWatts->setVisible(true);
        ui->label_acceptedPower->setVisible(true);
        ui->spinBox_rangePower->setVisible(true);
        ui->label_accptedWattsTxt->setVisible(true);
    }
    else { ///2- Progressive
        ui->spinBox_ftpStart->setVisible(true);
        ui->label_toFtpTxt->setVisible(true);
        ui->spinBox_ftpEnd->setVisible(true);
        ui->comboBox_FTPorWatts->setVisible(true);
        ui->label_acceptedPower->setVisible(true);
        ui->spinBox_rangePower->setVisible(true);
        ui->label_accptedWattsTxt->setVisible(true);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_comboBox_FTPorWatts_currentIndexChanged(int index)
{
    if (index == 0) { ///% FTP
        editingInWatts = false;
        ui->spinBox_ftpStart->setDecimals(2);
        ui->spinBox_ftpEnd->setDecimals(2);
        ui->spinBox_ftpStart->setMaximum(250);
        ui->spinBox_ftpEnd->setMaximum(250);
        on_spinBox_ftpStart_valueChanged(ui->spinBox_ftpStart->value());
        on_spinBox_ftpEnd_valueChanged(ui->spinBox_ftpEnd->value());

    }
    else { ///Watts
        editingInWatts = true;
        ui->spinBox_ftpStart->setDecimals(0);
        ui->spinBox_ftpEnd->setDecimals(0);
        ui->spinBox_ftpStart->setMaximum(999);
        ui->spinBox_ftpEnd->setMaximum(999);
        on_spinBox_ftpStart_valueChanged(ui->spinBox_ftpStart->value());
        on_spinBox_ftpEnd_valueChanged(ui->spinBox_ftpEnd->value());
    }
}



//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_spinBox_ftpStart_valueChanged(double arg1)
{
    // Convert to FTP value
    int ftp = account->FTP;
    if (editingInWatts) {
        myInterval.setTargetFTP_start(arg1/ftp);
    }
    else {
        myInterval.setTargetFTP_start(arg1/100);
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_spinBox_ftpEnd_valueChanged(double arg1)
{
    // Convert to FTP value
    int ftp = account->FTP;
    if (editingInWatts) {
        double ftpEnd = ((double)arg1)/ftp;
        myInterval.setTargetFTP_end(ftpEnd);
    }
    else {
        double ftpEnd = ((double)arg1)/100;
        myInterval.setTargetFTP_end(ftpEnd);
    }


}

//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_spinBox_rangePower_valueChanged(int arg1)
{
    myInterval.setTargetFTP_range(arg1);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_spinBox_leftBalance_valueChanged(int arg1)
{
    int restValue = 100-arg1;
    ui->spinBox_rightBalance->setValue(restValue);
    myInterval.setRightPowerTarget(restValue);
}
//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_spinBox_rightBalance_valueChanged(int arg1)
{
    int restValue = 100-arg1;
    ui->spinBox_leftBalance->setValue(restValue);
    myInterval.setRightPowerTarget(arg1);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_checkBox_balance_clicked(bool checked)
{

    if (checked) {
        ui->spinBox_leftBalance->setVisible(true);
        ui->spinBox_rightBalance->setVisible(true);
        ui->label_left->setVisible(true);
        ui->label_right->setVisible(true);
        //
        if (myInterval.getRightPowerTarget() == -1)
            on_spinBox_leftBalance_valueChanged(50);
    }
    else {
        ui->spinBox_leftBalance->setVisible(false);
        ui->spinBox_rightBalance->setVisible(false);
        ui->label_left->setVisible(false);
        ui->label_right->setVisible(false);
        myInterval.setRightPowerTarget(-1);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void PowerEditor::on_checkBox_testInterval_clicked(bool checked)
{
    myInterval.setTestInterval(checked);

}

void PowerEditor::on_pushButton_ok_clicked()
{
    emit endEdit();
}





void PowerEditor::on_pushButton_default_clicked()
{

    this->myInterval.setPowerStepType(Interval::StepType::NONE);
    this->myInterval.setTargetFTP_start(0.60);
    this->myInterval.setTargetFTP_end(0.60);
    this->myInterval.setTargetFTP_range(20);
    this->myInterval.setTestInterval(false);
    this->myInterval.setRightPowerTarget(-1);

    emit endEdit();
}



