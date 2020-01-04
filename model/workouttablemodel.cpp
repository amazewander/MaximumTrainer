#include "workouttablemodel.h"

#include <QDebug>
#include <QFont>
#include <QBrush>

#include "util.h"





WorkoutTableModel::WorkoutTableModel(QObject *parent) : QAbstractTableModel(parent) {

    qDebug() << "WorkoutTableModel";
    this->account = qApp->property("Account").value<Account*>();

    rowHovered = -2;
}


void WorkoutTableModel::changeRowHovered(int row) {
    this->rowHovered = row;
}




void WorkoutTableModel::addListWorkout(QList<Workout> lstWorkout) {

    beginResetModel();
    this->lstWorkout.append(lstWorkout);
    endResetModel();

    emit dataChanged();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkoutTableModel::updateWorkoutsMetrics() {


    for (int i=0; i<lstWorkout.size(); i++) {

        Workout workout = lstWorkout.at(i);
        workout.calculateWorkoutMetrics();

        QModelIndex idx = index(i, 0, QModelIndex());
        QVariant stored;
        stored.setValue(workout);
        setData(idx, stored, Qt::UserRole);
    }
    emit dataChanged();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Workout WorkoutTableModel::getWorkoutAtRow(const QModelIndex &index) {

    return (lstWorkout.at(index.row()));
}


// ------------------------------------------------------------------------------------------
int WorkoutTableModel::rowCount(const QModelIndex &parent) const {

    Q_UNUSED(parent);
    return lstWorkout.size();
}


// ------------------------------------------------------------------------------------------
int WorkoutTableModel::columnCount(const QModelIndex &parent) const {

    Q_UNUSED(parent);
    return 11;
}



//--------------------------------------------------------------------------------------------------------
bool WorkoutTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {


    if (role == Qt::UserRole) {
        Workout workout = value.value<Workout>();
        lstWorkout.replace(index.row(), workout);
    }

    //    emit dataChanged();
    return true;
}


// ------------------------------------------------------------------------------------------
QVariant WorkoutTableModel::data(const QModelIndex &index, int role) const {


    if (!index.isValid())
        return QVariant();

    if (index.row() >= lstWorkout.size() || index.row() < 0)
        return QVariant();


    Workout work = lstWorkout.at(index.row());


    /// TOOLTIP
    if (role == Qt::ToolTipRole) {
        return "<qt/>"+work.getDescription();
    }


    ///Background
    if (role == Qt::BackgroundColorRole && account->hashWorkoutDone.contains(work.getName())) {
        //        QColor colorCadenceShapeTarget(150,150,150);
        QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 125));
        linearGrad.setColorAt(1, Qt::white);
        linearGrad.setColorAt(0, QColor(150,150,150));

        return QBrush(linearGrad);
    }


    ///BlueName for UserCreatedWorkout
    if (role == Qt::TextColorRole && index.column() == 0 && account->enable_studio_mode && work.getWorkoutNameEnum() == Workout::MAP_TEST)
        return QVariant::fromValue(QColor(Qt::lightGray));
    else if (role == Qt::TextColorRole && index.column() == 0 && work.getWorkoutNameEnum() == Workout::USER_MADE) {
        return QVariant::fromValue(QColor(Qt::blue));
    }



    if (role == Qt::DisplayRole) {


        if (index.column() == 0)  {
            return work.getName();
        }
        else if(index.column() == 1) {
            return work.getPlan();
        }
        else if(index.column() == 2) {
            return work.getCreatedBy();
        }
        else if(index.column() == 3) {
            return work.getTypeToString();
        }
        else if(index.column() == 4) {
            if (work.getWorkoutNameEnum() == Workout::MAP_TEST)
                return ("-");
            return work.getDurationQTime().toString("h:mm");
        }
        else if(index.column() == 5) {
            if (work.getWorkoutNameEnum() == Workout::MAP_TEST)
                return ("-");
            return work.getMaxPowerPourcent()*100;
        }
        else if(index.column() == 6) {  //AP
            if (work.getWorkoutNameEnum() == Workout::MAP_TEST)
                return ("-");
            return qRound(work.getAveragePower());
        }
        else if(index.column() == 7) {  //NP
            if (work.getWorkoutNameEnum() == Workout::MAP_TEST)
                return ("-");
            return qRound(work.getNormalizedPower());
        }
        else if(index.column() == 8) {  //IF
            if (work.getWorkoutNameEnum() == Workout::MAP_TEST)
                return ("-");
            return QString::number( work.getIntensityFactor(), 'f', 2);
        }
        else if(index.column() == 9) {  //TSS
            if (work.getWorkoutNameEnum() == Workout::MAP_TEST)
                return ("-");
            return qRound(work.getTrainingStressScore());
        }

        else if(index.column() == 20) {
            return work.getWorkoutNameEnum();
        }

        /// custom paint in delegate
        //        else {
        //            return tr("column not defined");
        //        }
    }

    /// Graph
    //    if (index.column() == 6) {
    //        return QVariant();
    //    }

    /// Graph
    if (role == Qt::UserRole) {
        QVariant stored;
        stored.setValue(work);
        return stored;
    }

    return QVariant();
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkoutTableModel::deleteWorkoutWithName(QString name) {

    //TOFIX; Use hashtable instead QList, faster access
    for (int i=0; i<lstWorkout.size(); i++)
    {
        Workout workout = lstWorkout.at(i);
        if (workout.getName() == name) {
            lstWorkout.removeAt(i);
            break;
        }
    }
    emit dataChanged();
}

//---------------------------------------------------------------------
void WorkoutTableModel::deleteAllUserMadeWorkouts() {

    beginResetModel();
    QMutableListIterator<Workout> i(lstWorkout);
    while (i.hasNext()) {
        if (i.next().getWorkoutNameEnum() == Workout::USER_MADE) {
            i.remove();
        }
    }
    endResetModel();
}
//---------------------------------------------------------------------
void WorkoutTableModel::deleleteMapWorkout() {


    beginResetModel();
    QMutableListIterator<Workout> i(lstWorkout);
    while (i.hasNext()) {
        if (i.next().getWorkoutNameEnum() == Workout::MAP_TEST) {
            i.remove();
        }
    }
    endResetModel();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WorkoutTableModel::addWorkout(Workout w) {

    beginResetModel();
    lstWorkout.append(w);
    endResetModel();

    emit dataChanged();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool WorkoutTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    qDebug() << "REMOVE ROW";
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; ++row) {
        lstWorkout.removeAt(position);
    }
    endRemoveRows();
    emit dataChanged();
    return true;

}




// --------------------------------------------------------------------------------------------
QVariant WorkoutTableModel::headerData(int section, Qt::Orientation  orientation, int role) const {


    if (role == Qt::ToolTipRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return tr("Name");
        }
        else if (section == 1) {
            return tr("Plan");
        }
        else if (section == 2) {
            return tr("Creator");
        }
        else if (section == 3) {
            return tr("Type");
        }
        else if (section == 4) {
            return tr("Duration (h:mm)");
        }
        else if (section == 5) {
            return tr("Maximum %FTP");
        }
        else if (section == 6) {
            return tr("Average power");
        }
        else if (section == 7) {
            return tr("Normalized power®");
        }
        else if (section == 8) {
            return tr("Intensity Factor®");
        }
        else if (section == 9) {
            return tr("Training Stress Score®");
        }
        else if (section == 10) {
            return tr("Graph");
        }
        else {
            return tr("colum not defined");
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical) {
        return QString::number(section);
    }
    else {
        if (section == 0) {
            return tr("Name");
        }
        else if (section == 1) {
            return tr("Plan");
        }
        else if (section == 2) {
            return tr("Creator");
        }
        else if (section == 3) {
            return tr("Type");
        }
        else if (section == 4) {
            return tr("Duration");
        }
        else if (section == 5) {
            return tr("Max FTP");
        }
        else if (section == 6) {
            return tr("AP");
        }
        else if (section == 7) {
            return tr("NP");
        }
        else if (section == 8) {
            return tr("IF");
        }
        else if (section == 9) {
            return tr("TSS");
        }
        else if (section == 10) {
            return tr("Graph");
        }
        else {
            return tr("colum not defined");
        }
    }

}

