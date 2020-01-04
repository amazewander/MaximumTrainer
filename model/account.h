#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QtCore>
#include <powercurve.h>


//We use same syntax as mySql, so no camel-case here



class Account : public QObject
{
    Q_OBJECT

public:
    ~Account();
    Account(QObject *parent = 0);

    void saveNbSecShowInterval(int nbSec);
    void saveNbSecShowIntervalBefore(int nbSec);


    int id;
    int subscription_type_id;  //1 = Free, 2= Regular, 3=Studio

    QString email;
    QString password;
    QString session_mt_id;
    QDateTime session_mt_expire;

    QString first_name;
    QString last_name;
    QString display_name;


    int FTP;
    int LTHR;
    int minutes_rode;
    double weight_kg;
    bool weight_in_kg;
    int height_cm;

    PowerCurve powerCurve;
    int wheel_circ;
    double bike_weight_kg;
    int bike_type;


    //-------------------------- not in DB ----------------------
    double userCda;
    QString os;

    QString email_clean; //"blais.maxime@gmail.com --> blaismaxime"
    QSet<QString> hashWorkoutDone;
    QSet<QString> hashCourseDone;

    int nb_sec_show_interval;
    int nb_sec_show_interval_before;



    // -----------------------------------  Settings ----------------------------------------------------------------------
    int nb_ant_stick;
    int nb_user_studio;
    bool enable_studio_mode;
    bool use_pm_for_cadence;
    bool use_pm_for_speed;

    bool force_workout_window_on_top;
    bool show_included_workout;
    bool show_included_course;
    bool distance_in_km;
    QString strava_access_token;
    bool strava_private_upload;
    QString selfloops_user;
    QString selfloops_pw;

    QString training_peaks_access_token;
    QString training_peaks_refresh_token;
    bool training_peaks_public_upload;

    bool control_trainer_resistance;
    bool stop_pairing_on_found;
    int nb_sec_pairing;
    /* ----- */

    int last_index_selected_config_workout;
    int last_tab_sub_config_selected;
    QString tab_display[8];

    //-------------- Util functions -------------
    // Must Matches the DB identifier
    QString getTimerStr() const {
        return "Timer";
    }
    QString getHrStr() const {
        return "Hr";
    }
    QString getPowerStr() const {
        return "Power";
    }
    QString getCadenceStr() const {
        return "Cadence";
    }
    QString getPowerBalanceStr() const {
        return "PowerBal";
    }
    QString getSpeedStr() const {
        return "Speed";
    }
    QString getOxygenStr() const {
        return "Oxygen";
    }
    QString getInfoWorkoutStr() const{
        return "InfoWorkout";
    }
    int getNumberWidget() const {
        return 8;
    }
    //-------------- /Util functions -------------




    int start_trigger;
    int value_cadence_start;
    int value_power_start;
    int value_speed_start;

    bool show_hr_widget;
    bool show_power_widget;
    bool show_power_balance_widget;
    bool show_cadence_widget;
    bool show_speed_widget;
    bool show_calories_widget;
    bool show_oxygen_widget;
    bool use_virtual_speed;
    bool show_trainer_speed;

    int display_hr;
    int display_power;
    int display_power_balance;
    int display_cadence;

    bool show_timer_on_top;
    bool show_interval_remaining;
    bool show_workout_remaining;
    bool show_elapsed;
    int font_size_timer;

    int averaging_power;
    int offset_power;

    bool show_seperator_interval;
    bool show_grid;
    bool show_hr_target;
    bool show_power_target;
    bool show_cadence_target;
    bool show_speed_target;
    bool show_hr_curve;
    bool show_power_curve;
    bool show_cadence_curve;
    bool show_speed_curve;

    int display_video;

    /* ----- */
    int sound_player_vol;
    bool enable_sound;
    bool sound_interval;
    bool sound_pause_resume_workout;
    bool sound_achievement;
    bool sound_end_workout;

    bool sound_alert_power_under_target;
    bool sound_alert_power_above_target;
    bool sound_alert_cadence_under_target;
    bool sound_alert_cadence_above_target;
    //----










};
Q_DECLARE_METATYPE(Account*)

#endif // ACCOUNT_H
