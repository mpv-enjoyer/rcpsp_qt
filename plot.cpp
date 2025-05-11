#include "plot.h"

QCPBars *Plot::createBars(QString name, QColor color, QCustomPlot *plot)
{
    QCPBars *set = new QCPBars(plot->yAxis, plot->xAxis);
    set->setAntialiased(false);
    set->setStackingGap(0);
    set->setName(name);
    set->setPen(QColor::fromRgb(0, 0, 0, 0));
    set->setBrush(color);
    return set;
}

Plot::Plot(QCustomPlot* plot)
{
    _plot = plot;
    // set dark background gradient:
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(90, 90, 90));
    gradient.setColorAt(0.38, QColor(105, 105, 105));
    gradient.setColorAt(1, QColor(70, 70, 70));
    plot->setBackground(QBrush(gradient));

    // create empty bar chart objects:
    unable_set = createBars("", QColor::fromRgb(255, 255, 255, 0), plot);
    waiting_set = createBars("Поступление - Начало обслуживания", QColor::fromRgb(200, 200, 200), plot);
    executed_set = createBars("Начало обслуживания - Конец обслуживания", QColor::fromRgb(0, 0, 0), plot);
    ready_set = createBars("Конец обслуживания - Позднее время начала", QColor::fromRgb(200, 200, 200), plot);
    critical_set = createBars("Позднее время начала - Директивный срок", QColor::fromRgb(200, 150, 150), plot);
    overhead_wait_set = createBars("Директивный срок - Начало обслуживания (превышение срока)", QColor::fromRgb(140, 10, 10), plot);
    overhead_set = createBars("Начало обслуживания - Конец обслуживания (превышение срока)", QColor::fromRgb(255, 0, 0), plot);
    // stack bars on top of each other:
    waiting_set->moveAbove(unable_set);
    executed_set->moveAbove(waiting_set);
    ready_set->moveAbove(executed_set);
    critical_set->moveAbove(ready_set);
    overhead_wait_set->moveAbove(critical_set);
    overhead_set->moveAbove(overhead_wait_set);

    // prepare y axis with country labels:
    //auto y_axis_size = current_completed.size();
    //QVector<double> ticks(y_axis_size);
    //QVector<QString> labels(y_axis_size);

    /*for (auto i = 0; i < y_axis_size; i++)
    {
        ticks[i] = i + 1;
        QString current_left = "Работа №: ";
        current_left.append(QString().number(current_completed[i].job_id));
        current_left.append(", Рабочая группа: ");
        current_left.append(QString().number(current_completed[i].worker_group_id));
        current_left.append(", ID работника: ");
        current_left.append(QString().number(current_completed[i].worker_internal_id));
        current_left.append(", Начало: ");
        current_left.append(QString().number(current_completed[i].start));
        labels[i] = current_left;
    }*/

    plot->xAxis->setTickLabelRotation(60);
    plot->xAxis->setSubTicks(false);
    plot->xAxis->setTickLength(0, 4);
    plot->xAxis->setRange(0, 8);
    plot->xAxis->setBasePen(QPen(Qt::white));
    plot->xAxis->setTickPen(QPen(Qt::white));
    plot->xAxis->grid()->setVisible(true);
    plot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    plot->xAxis->setTickLabelColor(Qt::white);
    plot->xAxis->setLabelColor(Qt::white);

    // prepare y axis:
    //QSharedPointer<QCPAxisTickerText> _textTicker = QSharedPointer(new QCPAxisTickerText());
    _textTicker = QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText);
    plot->yAxis->setTicker(_textTicker);
    plot->yAxis->setRange(0, 12.1);
    plot->yAxis->setPadding(5); // a bit more space to the left border
    plot->yAxis->setLabel("Gantt chart (QCustomPlot)");
    plot->yAxis->setBasePen(QPen(Qt::white));
    plot->yAxis->setTickPen(QPen(Qt::white));
    plot->yAxis->setSubTickPen(QPen(Qt::white));
    plot->yAxis->grid()->setSubGridVisible(true);
    plot->yAxis->setTickLabelColor(Qt::white);
    plot->yAxis->setLabelColor(Qt::white);
    plot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    plot->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    // Add data:
    /*for (auto i = 0; i < y_axis_size; i++)
    {
        auto start_after = current_completed[i].job->get_start_after();
        auto waiting = current_completed[i].start - start_after;
        auto in_progress = current_completed[i].job->get_time_to_spend();
        auto critical = current_completed[i].job->get_critical_time() - start_after - waiting - in_progress;
        auto ready = current_completed[i].job->get_end_before() - start_after - waiting - in_progress - critical;
        start_after_data[i] = start_after;
        waiting_data[i] = waiting;
        in_progress_data[i] = in_progress;
        critical_data[i] = critical;
        ready_data[i] = ready;
    }*/
    //unable_set->setData(ticks, start_after_data);
    //waiting_set->setData(ticks, waiting_data);
    //executed_set->setData(ticks, in_progress_data);
    //critical_set->setData(ticks, critical_data);
    //ready_set->setData(ticks, ready_data);

    // setup legend:
    plot->legend->setVisible(true);
    plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    plot->legend->setBrush(QColor(255, 255, 255, 100));
    plot->legend->setBorderPen(Qt::NoPen);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->replot();
}

bool is_eless_int5(const int& o1, const int& o2, const int& o3, const int& o4, const int& o5)
{
    if (o1 > o2) return false;
    if (o2 > o3) return false;
    if (o3 > o4) return false;
    if (o4 > o5) return false;
    return true;
}

void Plot::updatePlot(const std::vector<ResultPair> &completed)
{
    auto y_axis_size = completed.size();
    QVector<double> ticks(y_axis_size);
    QVector<QString> labels(y_axis_size);
    for (auto i = 0; i < y_axis_size; i++)
    {
        ticks[i] = i + 1;
        QString current_left = "Требование №: ";
        current_left.append(QString().number(completed[i].job_id));
        current_left.append(", Группа станков: ");
        current_left.append(QString().number(completed[i].worker_group_id));
        current_left.append(", ID станка: ");
        current_left.append(QString().number(completed[i].worker_internal_id));
        current_left.append(", Начало: ");
        current_left.append(QString().number(completed[i].start));
        current_left.append(", Позднее начало: ");
        current_left.append(QString().number(completed[i].job->get_critical_time()));
        labels[i] = current_left;
    }
    QVector<double> start_after_data(y_axis_size);
    QVector<double> waiting_data(y_axis_size);
    QVector<double> in_progress_data(y_axis_size);
    QVector<double> critical_data(y_axis_size);
    QVector<double> ready_data(y_axis_size);
    QVector<double> overhead_wait_data(y_axis_size);
    QVector<double> overhead_data(y_axis_size);
    for (auto i = 0; i < y_axis_size; i++)
    {
        auto start_after = 0;
        auto waiting = 0;
        auto in_progress = 0;
        auto ready = 0;
        auto critical = 0;
        auto overhead_wait = 0;
        auto overhead = 0;

        auto real_begin_after = completed[i].job->get_start_after();
        auto real_begin = completed[i].start;
        auto real_end = completed[i].start + completed[i].job->get_time_to_spend();
        auto real_critical = completed[i].job->get_critical_time();
        auto real_end_before = completed[i].job->get_end_before();

        // Поступление < Начало < Конец < Позднее < Директивный
        if (is_eless_int5(real_begin_after, real_begin, real_end, real_critical, real_end_before))
        {
            start_after = real_begin_after;
            waiting = real_begin - real_begin_after;
            in_progress = real_end - real_begin;
            ready = real_critical - real_end;
            critical = real_end_before - real_critical;
        }
        // Поступление < Начало < Позднее < Конец < Директивный
        if (is_eless_int5(real_begin_after, real_begin, real_critical, real_end, real_end_before))
        {
            start_after = real_begin_after;
            waiting = real_begin - real_begin_after;
            in_progress = real_end - real_begin;
            critical = real_end_before - real_end;
            // ignore real_critical
        }
        // Поступление < Позднее < Начало < Конец < Директивный
        if (is_eless_int5(real_begin_after, real_critical, real_begin, real_end, real_end_before))
        {
            start_after = real_begin_after;
            waiting = real_begin - real_begin_after;
            in_progress = real_end - real_begin;
            critical = real_end_before - real_end;
            // ignore real_critical
        }
        // Поступление < Позднее < Начало < Директивный < Конец
        if (is_eless_int5(real_begin_after, real_critical, real_begin, real_end_before, real_end))
        {
            start_after = real_begin_after;
            waiting = real_begin - real_begin_after;
            in_progress = real_end_before - real_begin;
            overhead = real_end - real_end_before;
        }
        // Поступление < Позднее < Директивный < Начало < Конец
        if (is_eless_int5(real_begin_after, real_critical, real_end_before, real_begin, real_end))
        {
            start_after = real_begin_after;
            waiting = real_end_before - real_begin_after;
            overhead_wait = real_begin - real_end_before;
            overhead = real_end - real_begin;
        }
        // Позднее < Поступление < Начало < Конец < Директивный
        if (is_eless_int5(real_critical, real_begin_after, real_begin, real_end, real_end_before))
        {
            labels[i] += " !!!";
            start_after = real_begin_after;
            waiting = real_begin - real_begin_after;
            in_progress = real_end - real_begin;
            ready = real_end_before - real_end;
        }
        // Позднее < Поступление < Начало < Директивный < Конец
        if (is_eless_int5(real_critical, real_begin_after, real_begin, real_end_before, real_end))
        {
            labels[i] += " !!!";
            start_after = real_begin_after;
            waiting = real_begin - real_begin_after;
            in_progress = real_end_before - real_begin;
            overhead = real_end - real_end_before;
        }
        // Позднее < Поступление < Директивный < Начало < Конец
        if (is_eless_int5(real_critical, real_begin_after, real_end_before, real_begin, real_end))
        {
            labels[i] += " !!!";
            start_after = real_begin_after;
            waiting = real_end_before - real_begin_after;
            overhead_wait = real_begin - real_end_before;
            overhead = real_end - real_begin;
        }

        start_after_data[i] = start_after;
        waiting_data[i] = waiting;
        in_progress_data[i] = in_progress;
        critical_data[i] = critical;
        ready_data[i] = ready;
        overhead_wait_data[i] = overhead_wait;
        overhead_data[i] = overhead;
    }
    _textTicker->clear();
    _textTicker->addTicks(ticks, labels);
    unable_set->setData(ticks, start_after_data);
    waiting_set->setData(ticks, waiting_data);
    executed_set->setData(ticks, in_progress_data);
    critical_set->setData(ticks, critical_data);
    ready_set->setData(ticks, ready_data);
    overhead_wait_set->setData(ticks, overhead_wait_data);
    overhead_set->setData(ticks, overhead_data);
    _plot->yAxis->setRange(0, 10);
    _plot->xAxis->setRange(0, 10);
    _plot->replot();
}

WaitStatsPlot::WaitStatsPlot(QCustomPlot *plot)
{
    _plot = plot;
}

void WaitStatsPlot::updatePlot(const Stats& stats)
{
    std::size_t to_reserve = stats.wait_coeff.size();
    QVector<double> x;
    x.reserve(to_reserve);
    QVector<double> y;
    y.reserve(to_reserve);
    double max_x = 0;
    for (auto wait_coeff : stats.wait_coeff)
    {
        x.push_back(wait_coeff.first);
        y.push_back(wait_coeff.second);
        max_x = wait_coeff.first;
    }
    _plot->addGraph();
    _plot->graph(0)->setData(x, y);
    _plot->xAxis->setLabel("x");
    _plot->yAxis->setLabel("y");
    //_plot->xAxis->setRange(0, max_x);
    _plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    _plot->replot();
}

WorkStatsPlot::WorkStatsPlot(QCustomPlot *plot)
{
    _plot = plot;
}

void WorkStatsPlot::updatePlot(const Stats &stats)
{
    std::size_t to_reserve = stats.work_coeff.size();
    QVector<double> x;
    x.reserve(to_reserve);
    QVector<double> y;
    y.reserve(to_reserve);
    double max_x = 0;
    for (auto work_coeff : stats.work_coeff)
    {
        x.push_back(work_coeff.first);
        y.push_back(work_coeff.second);
        max_x = work_coeff.first;
    }
    _plot->addGraph();
    _plot->graph(0)->setData(x, y);
    _plot->xAxis->setLabel("x");
    _plot->yAxis->setLabel("y");
    //_plot->xAxis->setRange(0, max_x);
    _plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    _plot->replot();
}
