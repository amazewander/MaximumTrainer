#include "account.h"
#include <QDebug>


Account::~Account() {
    qDebug() << "Destructor account";
}



Account::Account(QObject *parent) : QObject(parent)  {
    display_name = settings.value("last_use", "default").toString();

    //
    settings.beginGroup("account_" + display_name);
    nb_sec_show_interval = settings.value("nb_sec_show_interval", 5 ).toInt();
    nb_sec_show_interval_before = settings.value("nb_sec_show_interval_before", 4 ).toInt();

    id = settings.value("id", -1).toInt();
    subscription_type_id = settings.value("subscription_type_id", 1).toInt();

    email = settings.value("email", "email").toString();
    password = settings.value("password", "pw").toString();

    first_name = settings.value("first_name", "first_name").toString();
    last_name = settings.value("last_name", "last_name").toString();

    FTP = settings.value("FTP", 150).toInt();
    LTHR = settings.value("LTHR", 150).toInt();
    minutes_rode = settings.value("minutes_rode", 0).toInt();
    weight_kg = settings.value("weight_kg", 70).toInt();
    weight_in_kg = settings.value("weight_in_kg", true).toBool();
    height_cm = settings.value("height_cm", 170).toInt();

    powerCurve = PowerCurve();
    wheel_circ = settings.value("wheel_circ", 2100).toInt();
    bike_weight_kg = settings.value("bike_weight_kg", 9).toInt();
    bike_type = settings.value("bike_type", 2).toInt();


    userCda = settings.value("userCda", 0.35).toDouble();


    control_trainer_resistance = settings.value("trainerResistance", true).toBool();
    settings.endGroup();




    // -----------------------------------  Settings ----------------------------------------------------------------------
    settings.beginGroup("setting_" + display_name);
    nb_ant_stick = 1;
    nb_user_studio = 3;
    enable_studio_mode = false;
    use_pm_for_cadence = false;
    use_pm_for_speed = false;


    force_workout_window_on_top = false;
    show_included_workout = true;
    show_included_course = true;
    distance_in_km = true;
    strava_access_token = "";
    strava_private_upload = false;
    training_peaks_access_token = settings.value("training_peaks_access_token", "").toString();
    training_peaks_refresh_token = settings.value("training_peaks_refresh_token", "").toString();
    training_peaks_public_upload = false;
    selfloops_user = "";
    selfloops_pw = "";
    stop_pairing_on_found = true;
    nb_sec_pairing = 2;
    /* ----- */

    last_index_selected_config_workout = 0;
    last_tab_sub_config_selected = 0;
    tab_display[0] = "Timer";
    tab_display[1] = "Power";
    tab_display[2] = "Cadence";
    tab_display[3] = "PowerBal";
    tab_display[4] = "Hr";
    tab_display[5] = "Speed";
    tab_display[6] = "InfoWorkout";
    tab_display[7] = "Oxygen";


    start_trigger = 0;
    value_cadence_start = 40;
    value_power_start = 120;
    value_speed_start = 20;


    show_hr_widget = true;
    show_power_widget = true;
    show_power_balance_widget = true;
    show_cadence_widget = true;
    show_speed_widget = true;
    show_calories_widget = true;
    show_oxygen_widget = true;
    use_virtual_speed = true;
    show_trainer_speed = true;

    display_hr = 1;
    display_power = 2;
    display_power_balance = 1;
    display_cadence = 1;

    show_timer_on_top = false;
    show_interval_remaining = true;
    show_workout_remaining = false;
    show_elapsed = true;
    font_size_timer = 26;

    averaging_power = 2;
    offset_power = 0;



    show_seperator_interval = true;
    show_grid = false;
    show_hr_target = true;
    show_power_target = true;
    show_cadence_target = true;
    show_speed_target = true;
    show_hr_curve = true;
    show_power_curve = true;
    show_cadence_curve = true;
    show_speed_curve = true;
    display_video = 0;

    /* ----- */
    sound_player_vol = 100;
    enable_sound = true;
    sound_interval = true;
    sound_pause_resume_workout = true;
    sound_achievement = true;
    sound_end_workout = true;

    sound_alert_power_under_target = false;
    sound_alert_power_above_target = false;
    sound_alert_cadence_under_target = false;
    sound_alert_cadence_above_target = false;

    settings.endGroup();

    //-------------------------- not in DB ----------------------
#ifdef Q_OS_WIN32
    os = "win";
#endif
#ifdef Q_OS_MAC
    os = "mac";
#endif

    email_clean = "user1";
    QVariant v = settings.value("account_" + display_name+"/hashWorkoutDone");
    QList<QString> list = v.value<QList<QString>>();
    hashWorkoutDone = QSet<QString>(list.begin(), list.end());

    v = settings.value("account_" + display_name+"/hashCourseDone");
    list = v.value<QList<QString>>();
    hashCourseDone = QSet<QString>(list.begin(), list.end());
    //------------------------------

}

void Account::setControl_trainer_resistance(bool value)
{
    control_trainer_resistance = value;
    settings.setValue("account_" + display_name+"/trainerResistance", value);
}


void Account::saveNbSecShowInterval(int nbSec) {

    nb_sec_show_interval = nbSec;

    settings.beginGroup("account_" + display_name);
    settings.setValue("nb_sec_show_interval", nb_sec_show_interval);

    settings.endGroup();
}

void Account::saveNbSecShowIntervalBefore(int nbSec) {

    nb_sec_show_interval_before = nbSec;

    settings.beginGroup("account_" + display_name);
    settings.setValue("nb_sec_show_interval_before", nb_sec_show_interval_before);

    settings.endGroup();
}

int Account::getFTP(){
    return this->FTP;
}

void Account::setFTP(int value){
    this->FTP = value;
    settings.setValue("account_" + display_name+"/FTP", value);
}

int Account::getLTHR(){
    return this->LTHR;
}

void Account::setLTHR(int value){
    this->LTHR = value;
    settings.setValue("account_" + display_name+"/LTHR", value);
}

bool Account::getWeightInKg(){
    return this->weight_in_kg;
}
void Account::setWeightInKg(bool isKg){
    this->weight_in_kg = isKg;
    settings.setValue("account_" + display_name+"/weight_in_kg", isKg);
}

double Account::getWeightKg(){
    return this->weight_kg;
}

void Account::setWeightKg(double weightKg){
    this->weight_kg = weightKg;
    settings.setValue("account_" + display_name+"/weight_kg", weightKg);
}

int Account::getHeightCm(){
    return this->height_cm;
}

void Account::setHeightCm(int height){
    this->height_cm = height;
    settings.setValue("account_" + display_name+"/height_cm", height);
}

double Account::getUserCda(){
    return this->userCda;
}

void Account::setUserCda(double cda){
    this->userCda = cda;
    settings.setValue("account_" + display_name+"/userCda", cda);
}

int Account::getWheelCirc(){
    return this->wheel_circ;
}

void Account::setWheelCirc(int size_mm){
    this->wheel_circ = size_mm;
    settings.setValue("account_" + display_name+"/wheel_circ", size_mm);
}

double Account::getBikeWeightKg(){
    return this->bike_weight_kg;
}

void Account::setBikeWeightKg(double kg){
    this->bike_weight_kg = kg;
    settings.setValue("account_" + display_name+"/bike_weight_kg", kg);
}

int Account::getBikeType(){
    return this->bike_type;
}

void Account::setBikeType(int bikeType){
    this->bike_type = bikeType;
    settings.setValue("account_" + display_name+"/bike_type", bikeType);
}

QString Account::getDisplayName(){
    return this->display_name;
}

void Account::setDisplayName(QString name){
    this->display_name = name;
    settings.setValue("account_" + display_name+"/display_name", name);
    settings.setValue("last_use", name);
}



bool Account::getEnable_studio_mode() const
{
    return enable_studio_mode;
}

QString Account::getTraining_peaks_access_token() const
{
    return training_peaks_access_token;
}

void Account::setTraining_peaks_access_token(const QString &value)
{
    training_peaks_access_token = value;
    settings.setValue("setting_" + display_name+"/training_peaks_access_token", value);
}

QString Account::getTraining_peaks_refresh_token() const
{
    return training_peaks_refresh_token;
}

void Account::setTraining_peaks_refresh_token(const QString &value)
{
    training_peaks_refresh_token = value;
    settings.setValue("setting_" + display_name+"/training_peaks_refresh_token", value);
}

int Account::getMinutes_rode() const
{
    return minutes_rode;
}

void Account::setMinutes_rode(int value)
{
    minutes_rode = value;
    settings.setValue("account_" + display_name+"/minutes_rode", value);
}

QSet<QString> Account::getHashWorkoutDone() const
{
    return hashWorkoutDone;
}

void Account::setHashWorkoutDone(const QSet<QString> &value)
{
    hashWorkoutDone = value;
    saveHashWorkoutDone();
}

void Account::saveHashWorkoutDone(){
    //QVariant v;
    //v.setValue(hashWorkoutDone.values());
    settings.setValue("account_" + display_name+"/hashWorkoutDone", QVariant(hashWorkoutDone.values()));
}

void Account::removeHashWorkoutDone(QString name){
    hashWorkoutDone.remove(name);
    saveHashWorkoutDone();
}

void Account::insertHashWorkoutDone(QString name){
    hashWorkoutDone.insert(name);
    saveHashWorkoutDone();
}

int Account::getNb_sec_show_interval() const
{
    return nb_sec_show_interval;
}

int Account::getNb_sec_show_interval_before() const
{
    return nb_sec_show_interval_before;
}

QSet<QString> Account::getHashCourseDone() const
{
    return hashCourseDone;
}

void Account::setHashCourseDone(const QSet<QString> &value)
{
    hashCourseDone = value;
    QVariant v;
    v.setValue(value.values());
    settings.setValue("account_" + display_name+"/hashCourseDone", v);
}



