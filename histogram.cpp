#include "histogram.h"

Histogram::Histogram()
{


    for(int i = 0; i<256; i++){
        categories << QString::number(i);

    }
    valuesSet = new QBarSet("grayscale");
    //valuesSet->setColor(QColor(0,0,0, 255));
    barSeries = new QBarSeries();
    histogram = new QChart();
    axisX = new QBarCategoryAxis();
    axisY = new QValueAxis();

    for(int i = 0; i<256; i++){
        *valuesSet << i;

    }
    barSeries->append(valuesSet);
    histogram->addSeries(barSeries);
    histogram->setAnimationOptions(QChart::NoAnimation);
    histogram->legend()->hide();

    axisX->append(categories);


    QLinearGradient plotAreaGradient;
    plotAreaGradient.setStart(QPointF(0, 1));
    plotAreaGradient.setFinalStop(QPointF(1, 0));
    plotAreaGradient.setColorAt(0.0, QRgb(0x555555));
    plotAreaGradient.setColorAt(1.0, QRgb(0x111111));
    plotAreaGradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    histogram->setPlotAreaBackgroundBrush(plotAreaGradient);
    histogram->setPlotAreaBackgroundVisible(true);
    histogram->setBackgroundVisible(false);

    histogram->setContentsMargins(0, 0, 0, 0);
    histogram->layout()->setContentsMargins(2, 1, 5, 5);
    histogram->setMargins({0,0,0,0});

    histogram->setBackgroundRoundness(0);


  }


void Histogram::SetChartData(QCLVector<int> data)
{
    int value = 0;
    for(int i = 0; i<256; i++){
        data.read(&value, 1, i);
        valuesSet->replace(i, value/1000);
    }

    histogram->removeSeries(barSeries);
    histogram->addSeries(barSeries);
    //chartView->repaint();
    //chartView->update();


}
