#ifndef WORKOUTPLOTCADENCE_H
#define WORKOUTPLOTCADENCE_H

#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "workout.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_histogram.h"
#include "shapefactory.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_textlabel.h"
#include "qwt_plot_shapeitem.h"
#include "markeritem.h"
#include "zoneitem.h"
#include "iconitem.h"

#include <QGridLayout>
#include "account.h"

#include "myqwtplotpicker.h"


class QwtPlotCurve;
class QwtPlotDirectPainter;


class WorkoutPlotZoomer : public QwtPlot
{
    Q_OBJECT

public:

    enum GRAPH_TYPE
    {
        POWER,
        CADENCE,
        HEART_RATE,
    };
    ~WorkoutPlotZoomer();
    WorkoutPlotZoomer(QWidget *parent = 0);
    void setWorkoutData(Workout workout, GRAPH_TYPE graph, bool firstInit);
    void setUserData(double FTP, double LTHR);
    void setStopped(bool b);
    void setPosition(double timeMsec);
    void addMarkerInterval(double time);





public slots:
    void updateCurve(double, int);
    void moveIntervalTime(double timeNow);

    void updateTextLabelValue(int value);
    void targetChanged(double percentageTarget, int range );




private :
    void moveIntervalTarget(double target, int range);

    void init(GRAPH_TYPE graph, bool firstInit);
    void clearPlot();
    void drawGraphIntervalsCadence();
    void drawGraphIntervalsPower();
    void drawGraphIntervalsHeartrate();
    void addShapeFromPoints(const QString &title, const QColor &color, QList<QPointF> lstPoints, int positionZ, bool antiliasing);






private:
    Workout workout;
    double FTP;
    double LTHR;

    GRAPH_TYPE type;
    int extraAfterRange;


    MyQwtPlotPicker *d_picker;

    QList<QwtPlotMarker*> lstPlotMarket;
    QList<QwtPlotShapeItem*> lstTargetShape;

    MarkerItem *markerLineMiddle;
    ZoneItem *zone1;
    ZoneItem *zone2;
    IconItem *icon ;


    int target;
    int targetRange;
    bool noTarget;
    int leftLow;
    int leftHigh;

    QwtText valueQwtText;
    QwtPlotTextLabel *qwtTextLabel;
    bool isStopped;



    QwtPlotCurve *curve;
    QVector<QPointF> samples;

    QwtInterval d_interval;



};

#endif // WORKOUTPLOTCADENCE_H
