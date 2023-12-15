#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QWidget>
#include <QtCharts>
#include "algorithm.h"
#include "contentwidget.h"
#include <QtDebug>
//#include <QColor>

class ChartView: public ContentWidget
{
    QValueAxis *axisX = nullptr;
    QChart* chart;
    QBarSet* unable_set;
    QBarSet* waiting_set;
    QBarSet* executed_set;
    QBarSet* critical_set;
    QBarSet* ready_set;
    QBarCategoryAxis* axisY;
    QHorizontalStackedBarSeries* series = nullptr;
public:
    ChartView(QWidget* base, std::vector<ResultPair>  result = std::vector<ResultPair>());
    void reload(const std::vector<ResultPair>& result);
};

#endif // CHARTVIEW_H
