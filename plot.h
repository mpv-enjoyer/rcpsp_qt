#ifndef PLOT_H
#define PLOT_H

#include "qcustomplot.h"
#include "algorithm.h"
#include <QWidgetData>
#include <QFontDatabase>
#include <QSharedPointer>

class Plot
{
    QCPBars *unable_set;
    QCPBars *waiting_set;
    QCPBars *executed_set;
    QCPBars *critical_set;
    QCPBars *ready_set;
    QCPBars *overhead_wait_set;
    QCPBars *overhead_set;
    QCustomPlot* _plot;
    QSharedPointer<QCPAxisTickerText> _textTicker;
    QCPBars* createBars(QString name, QColor color, QCustomPlot* plot);
public:
    Plot() {}
    Plot(QCustomPlot* plot);
    void updatePlot(const std::vector<ResultPair>& completed);
};

class WaitStatsPlot
{
    QCustomPlot* _plot;
public:
    const char* label = "коэффициент ожидания перед началом";
    WaitStatsPlot() { }
    WaitStatsPlot(QCustomPlot* plot);
    void updatePlot(const Stats &stats);
};

class WorkStatsPlot
{
    QCustomPlot* _plot;
public:
    const char* label = "коэффициент работы";
    WorkStatsPlot() { }
    WorkStatsPlot(QCustomPlot* plot);
    void updatePlot(const Stats &stats);
};

#endif // PLOT_H
