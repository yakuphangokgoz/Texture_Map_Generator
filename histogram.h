#ifndef HISTOGRAM_H
#define HISTOGRAM_H


#include <QtCharts>
#include <QChart>
#include <QBarSet>
#include "qclvector.h"
#include <QApplication>

class Histogram
{
public:
    QStringList categories;
    QBarSet *valuesSet;
    QBarSeries *barSeries;
    QChart *histogram;
    QBarCategoryAxis *axisX;
    QValueAxis *axisY;

    Histogram();
    void SetChartData(QCLVector<int> data);

};

#endif // HISTOGRAM_H
