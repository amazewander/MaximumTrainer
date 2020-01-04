#ifndef RADIOTABLEMODEL_H
#define RADIOTABLEMODEL_H

#include <QAbstractTableModel>

#include "radio.h"


class RadioTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    RadioTableModel(QObject *parent = 0);


    void addListRadio(QList<Radio> lstRadio);
    Radio getRadioAtRow(const QModelIndex &index);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role) const;



    void setActiveIndex(QModelIndex index);





public slots :
    void changeRowHovered(int row);



private:
    QList<Radio> lstRadio;
    int rowHovered;

    QModelIndex activeIndex;


};

#endif // RADIOTABLEMODEL_H















