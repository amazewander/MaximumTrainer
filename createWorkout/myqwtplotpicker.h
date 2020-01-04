#ifndef MYQWTPLOTPICKER_H
#define MYQWTPLOTPICKER_H

#include "qwt_plot_picker.h"


class MyQwtPlotPicker : public QwtPlotPicker
{
    Q_OBJECT
public:

    explicit MyQwtPlotPicker( int xAxis, int yAxis,
        RubberBand rubberBand, DisplayMode trackerMode, QWidget * );



signals:

public slots:

protected :
    virtual QwtText trackerTextF( const QPointF & ) const;


};

#endif // MYQWTPLOTPICKER_H
