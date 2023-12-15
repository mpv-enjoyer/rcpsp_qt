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
    //chartview->chart()->series()
            //cview->chart()->removeAllSeries();
        /*auto barset = new QBarSet("sample data");
        *barset << 1;
        *barset << 2;
        auto currentseries = new QHorizontalStackedBarSeries;
        currentseries->append(barset);
        cview->chart()->addSeries(currentseries);*/
    unable_set  = new QBarSet("Не может выполняться");
    waiting_set = new QBarSet("Ожидает");
    executed_set  = new QBarSet("Выполняется");
    critical_set = new QBarSet("Критическое время");
    ready_set = new QBarSet("Остаток");

    axisY->clear();

    for (auto iter = result.begin(); iter != result.end(); iter++)
    {
        QString current_left = "Job ";
        current_left.append(QString().number(iter->job_id));
        current_left.append(", Worker Group ");
        current_left.append(QString().number(iter->worker_group_id));
        current_left.append(", Worker Internal ");
        current_left.append(QString().number(iter->worker_internal_id));
        current_left.append(", Start at ");
        current_left.append(QString().number(iter->start));
       // left_list.append(current_left);
        //current_left.append(QString().number());
        axisY->append(current_left);
        int start_after = iter->job->get_start_after();
        int waiting = iter->start - start_after;
        int in_progress = iter->job->get_time_to_spend();
        int critical = iter->job->get_critical_time() - start_after - waiting - in_progress;
        int ready = iter->job->get_end_before() - start_after - waiting - in_progress - critical;
       // QDebug() << start_after << waiting << in_progress << critical << ready;
        //series->append(start_after, waiting, in_progress, critical, ready);

        //series->append(barset);
        //series->barSets()[0]->append(start_after);
        //series->barSets()[1]->append(waiting);
        //series->barSets()[2]->append(in_progress);
        //series->barSets()[3]->append(critical);
        //series->barSets()[4]->append(ready);
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
    cview->chart()->addSeries(currentseries);
    //axisY = new QBarCategoryAxis;
    //chart->addAxis(axisY, Qt::AlignLeft);
    currentseries->attachAxis(axisY);
    //auto axisX = new QValueAxis;
   // chart->addAxis(axisX, Qt::AlignBottom);
    currentseries->attachAxis(axisX);
    axisX->setTickCount(15);
}
