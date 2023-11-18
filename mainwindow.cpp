#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Job* first_job = new Job(3, 0, 4);
    Job* second_job = new Job(2, 0, 7);
    Job* third_job = new Job(5, 0, 2);
    all_jobs.push_back(first_job);
    all_jobs.push_back(second_job);
    all_jobs.push_back(third_job);
    Plan common_plan = Plan({{8, 2}});
    Worker* first_worker = new Worker(common_plan);
    Worker* second_worker = new Worker(common_plan);
    WorkerGroup* first_worker_group = new WorkerGroup();
    first_worker_group->add_worker(first_worker);
    first_worker_group->add_worker(second_worker);
    JobGroup* first_job_group = new JobGroup({first_job, second_job, third_job}, 3, __INT_MAX__);
    algorithm.add_job_group(first_job_group, first_worker_group);
    algorithm.run();

    ui->setupUi(this);

    setupPlot(ui->plot);

    updatePlot(ui->plot, 10);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updatePlot(QCustomPlot *customPlot, int overall_time)
{
    customPlot->clearItems();
    customPlot->addGraph();
    std::vector<ResultPair> current_completed = algorithm.get_completed();
    for (int i = 0; i < current_completed.size(); i++)
    {
        ResultPair current_pair = current_completed[i];
        auto found = std::find(all_jobs.begin(), all_jobs.end(), current_pair.job);
        int index = std::distance(all_jobs.begin(), found);
        QCPItemRect* rect = new QCPItemRect( customPlot );
        rect->topLeft->setCoords(QPointF(current_pair.start, index));
        switch (index)
        {
        case 0:
            rect->setBrush(QBrush(QColor("magenta")));
            break;
        case 1:
            rect->setBrush(QBrush(QColor("cyan")));
            break;
        case 2:
            rect->setBrush(QBrush(QColor("green")));
        }
        rect->bottomRight->setCoords(QPointF(current_pair.start + current_pair.job->get_time_to_spend(), index - 1));
    }
    /*customPlot->clearItems();
    customPlot->addGraph();
    QVector<double> x(overall_time + 1), y(overall_time + 1);
    std::vector<int> drawn;
    for (int i = 0; i < overall_time; i++)
    {
        int current = 0;
        for (int j = 0; j < visual_jobs.size(); j++)
        {
            int begin = visual_jobs[j]->get_begin_at();
            int end = begin + visual_jobs[j]->get_time_to_spend();
            if (begin <= i && i < end)
            {
                if (std::find(drawn.begin(), drawn.end(), j) == drawn.end())
                {
                    drawn.push_back(j);
                }
                QCPItemRect* rect=new QCPItemRect( ui->plot );
                rect->topLeft->setCoords(QPointF(i,current));
                rect->setBrush(QBrush(visual_jobs[j]->get_color()));
                current += visual_jobs[j]->get_want_renewable();
                rect->bottomRight->setCoords(QPointF(i+1,current));
            }
        }
        x[i] = i;
        y[i] = current;
    }
    //customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    //customPlot->graph(0)->setData(x, y);
    customPlot->xAxis->setLabel("time");
    customPlot->yAxis->setLabel("renew");
    customPlot->xAxis->setRange(0, overall_time);
    customPlot->yAxis->setRange(0, renewables);
    for (int i = 0; i < previous_labels.size(); i++)
    {
        if (std::find(drawn.begin(), drawn.end(), previous_labels[i]->get_tag()) == drawn.end())
        {
            delete previous_labels[i];
            previous_labels.erase(previous_labels.begin() + i);
            continue;
        }
    }
    std::vector<int> previous_tags;
    for (int i = 0; i < previous_labels.size(); i++)
    {
        previous_tags.push_back(previous_labels[i]->get_tag());
    }
    for (int i = 0; i < drawn.size(); i++)
    {
        if (std::find(previous_tags.begin(), previous_tags.end(), drawn[i]) == previous_tags.end())
        {
            QPalette label_palette;
            //label_palette.setColor(QPalette::WindowText, Qt::black);
            label_palette.setColor(QPalette::WindowText, visual_jobs[drawn[i]]->get_color());
            ColorLabel* new_label = new ColorLabel(drawn[i]);
            ui->verticalLayout->addWidget(new_label);
            previous_labels.push_back(new_label);
            new_label->setPalette(label_palette);
            new_label->setText(QString::number(drawn[i]) + " job is assigned");
        }
    }*/
}

//Thanks to https://www.qcustomplot.com/index.php/support/forum/2213
void MainWindow::setupPlot(QCustomPlot *customPlot)
{
    customPlot->setInteraction(QCP::iRangeDrag, true);
    customPlot->setInteraction(QCP::iRangeZoom, true);

    QCPItemRect* rect=new QCPItemRect( ui->plot );
    rect->topLeft->setCoords(QPointF(0,100));
    rect->bottomRight->setCoords(QPointF(100,0));
    rect->setBrush(QBrush(QColor("floralwhite")));
    customPlot->xAxis->setLabel("time");
    customPlot->yAxis->setLabel("workers");
}

//void MainWindow::assignJob(int index, int time)
//{
    //AssignedJob* assigned = new AssignedJob(*(unassigned_jobs[index]), time);
    //visual_jobs.push_back(new VisualJob(*assigned));
    //delete unassigned_jobs[index];
    //unassigned_jobs.erase(unassigned_jobs.begin() + index);
//}
