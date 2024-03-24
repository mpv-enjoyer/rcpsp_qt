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
    waiting_set = createBars("Поступление работы - Начало обслуживания", QColor::fromRgb(200, 200, 200), plot);
    executed_set = createBars("Начало обслуживания - Конец обслуживания", QColor::fromRgb(0, 0, 0), plot);
    critical_set = createBars("Конец обслуживания - Позднее время начала", QColor::fromRgb(200, 150, 150), plot);
    ready_set = createBars("Позднее время начала - Директивный срок", QColor::fromRgb(200, 200, 200), plot);
    // stack bars on top of each other:
    waiting_set->moveAbove(unable_set);
    executed_set->moveAbove(waiting_set);
    critical_set->moveAbove(executed_set);
    ready_set->moveAbove(critical_set);
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

void Plot::updatePlot(const std::vector<ResultPair> &completed)
{
    auto y_axis_size = completed.size();
    QVector<double> ticks(y_axis_size);
    QVector<QString> labels(y_axis_size);
    for (auto i = 0; i < y_axis_size; i++)
    {
        ticks[i] = i + 1;
        QString current_left = "Работа №: ";
        current_left.append(QString().number(completed[i].job_id));
        current_left.append(", Рабочая группа: ");
        current_left.append(QString().number(completed[i].worker_group_id));
        current_left.append(", ID работника: ");
        current_left.append(QString().number(completed[i].worker_internal_id));
        current_left.append(", Начало: ");
        current_left.append(QString().number(completed[i].start));
        labels[i] = current_left;
    }
    QVector<double> start_after_data(y_axis_size);
    QVector<double> waiting_data(y_axis_size);
    QVector<double> in_progress_data(y_axis_size);
    QVector<double> critical_data(y_axis_size);
    QVector<double> ready_data(y_axis_size);
    for (auto i = 0; i < y_axis_size; i++)
    {
        auto start_after = completed[i].job->get_start_after();
        auto waiting = completed[i].start - start_after;
        auto in_progress = completed[i].job->get_time_to_spend();
        auto critical = completed[i].job->get_critical_time() - start_after - waiting - in_progress;
        auto ready = completed[i].job->get_end_before() - start_after - waiting - in_progress - critical;
        start_after_data[i] = start_after;
        waiting_data[i] = waiting;
        in_progress_data[i] = in_progress;
        critical_data[i] = critical;
        ready_data[i] = ready;
    }
    _textTicker->clear();
    _textTicker->addTicks(ticks, labels);
    unable_set->setData(ticks, start_after_data);
    waiting_set->setData(ticks, waiting_data);
    executed_set->setData(ticks, in_progress_data);
    critical_set->setData(ticks, critical_data);
    ready_set->setData(ticks, ready_data);
    _plot->replot();
}
