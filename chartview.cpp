#include "chartview.h"

ChartView::ChartView(QWidget* base, std::vector<ResultPair>  result) : ContentWidget(base)
{
    unable_set  = new QBarSet("Не может выполняться");
    waiting_set = new QBarSet("Ожидает");
    executed_set  = new QBarSet("Выполняется");
    critical_set = new QBarSet("Критическое время");
    ready_set = new QBarSet("Остаток");
    QStringList left_list = QStringList();

    for (auto iter = result.begin(); iter != result.end(); iter++)
    {
        QString current_left = "Работа №: ";
        current_left.append(QString().number(iter->job_id));
        current_left.append(", Рабочая группа: ");
        current_left.append(QString().number(iter->worker_group_id));
        current_left.append(", ID работника: ");
        current_left.append(QString().number(iter->worker_internal_id));
        current_left.append(", Начало: ");
        current_left.append(QString().number(iter->start));
        left_list.append(current_left);
        //current_left.append(QString().number());
        //axisY->append(current_left);
        int start_after = iter->job->get_start_after();
        int waiting = iter->start - start_after;
        int in_progress = iter->job->get_time_to_spend();
        int critical = iter->job->get_critical_time() - start_after - waiting - in_progress;
        int ready = iter->job->get_end_before() - start_after - waiting - in_progress - critical;
       // QDebug() << start_after << waiting << in_progress << critical << ready;
        *unable_set << start_after;
        *waiting_set << waiting;
        *executed_set << in_progress;
        *critical_set << critical;
        *ready_set << ready;
    }
    series = new QHorizontalStackedBarSeries;
    //std::vector<ResultPair> current_completed = algorithm.get_completed();//QHorizontalStackedBarSeries;
    series->append(unable_set);
    series->append(waiting_set);
    series->append(executed_set);
    series->append(critical_set);
    series->append(ready_set);
    chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Диаграмма Ганта");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    //QStringList categories {"Jan", "Feb", "Mar", "Apr", "May", "Jun"};
    axisY = new QBarCategoryAxis;
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    axisX = new QValueAxis;
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
   // axisX->setMin(0);
    //axisX->setTickCount(10);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    createDefaultChartView(chart);
    auto cview = defaultChartView();
    cview->chart()->removeAllSeries();
    //chartview = new QChartView(base);
    //chartview->setChart(chart);
   // chartview->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    //chartview->setBaseSize(500, 500);
    //chartview->setMinimumSize(500, 500);
    //chartview->setRubberBand(QChartView::RectangleRubberBand);
}

void ChartView::reload(const std::vector<ResultPair>&  result)
{
    auto cview = defaultChartView();
    unable_set  = new QBarSet("");//("Нулевой момент времени - Поступление работы");
    waiting_set = new QBarSet("Поступление работы - Начало обслуживания");
    executed_set  = new QBarSet("Начало обслуживания - Конец обслуживания");
    critical_set = new QBarSet("Конец обслуживания - Позднее время начала");
    ready_set = new QBarSet("Позднее время начала - Директивный срок");

    unable_set->setColor(QColor::fromRgb(255, 255, 255));
    waiting_set->setColor(QColor::fromRgb(200, 200, 200));
    executed_set->setColor(QColor::fromRgb(0, 0, 0));
    critical_set->setColor(QColor::fromRgb(200, 150, 150));
    ready_set->setColor(QColor::fromRgb(200, 200, 200));

    axisY->clear();

    for (auto iter = result.begin(); iter != result.end(); iter++)
    {
        QString current_left = "Работа №";
        current_left.append(QString().number(iter->job_id));
        //current_left.append(", Рабочая группа: ");
        //current_left.append(QString().number(iter->worker_group_id));
        current_left.append(", Работник №");
        current_left.append(QString().number(iter->worker_internal_id));
        current_left.append(", Начало: ");
        current_left.append(QString().number(iter->start));
        axisY->append(current_left);
        int start_after = iter->job->get_start_after();
        int waiting = iter->start - start_after;
        int in_progress = iter->job->get_time_to_spend();
        int critical = iter->job->get_critical_time() - start_after - waiting - in_progress;
        int ready = iter->job->get_end_before() - start_after - waiting - in_progress - critical;
        *unable_set << start_after;
        *waiting_set << waiting;
        *executed_set << in_progress;
        *critical_set << critical;
        *ready_set << ready;
    }
    auto currentseries = new QHorizontalStackedBarSeries;
    currentseries->append(unable_set);
    currentseries->append(waiting_set);
    currentseries->append(executed_set);
    currentseries->append(critical_set);
    currentseries->append(ready_set);
    currentseries->setBarWidth(1);
    cview->chart()->addSeries(currentseries);
    //axisY = new QBarCategoryAxis;
    //chart->addAxis(axisY, Qt::AlignLeft);
    currentseries->attachAxis(axisY);
    //auto axisX = new QValueAxis;
   // chart->addAxis(axisX, Qt::AlignBottom);
    currentseries->attachAxis(axisX);
    //axisX->setTickCount(15);
    axisX->setTickInterval(10);
    axisX->setTickAnchor(0);
    axisX->setTickType(QValueAxis::TicksDynamic);
    //axisX->applyNiceNumbers();
    connect(axisX, SIGNAL(rangeChanged(qreal,qreal)), this, SLOT(rangeChanged(qreal,qreal)));
}

void ChartView::rangeChanged(qreal min, qreal max)
{
    qreal diff = max - min;
    qreal interval = diff / 15;
    axisX->setTickInterval(interval);
}
