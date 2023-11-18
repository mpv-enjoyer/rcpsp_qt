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
        rect->topLeft->setCoords(QPointF(current_pair.start, index + 1));
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
        rect->bottomRight->setCoords(QPointF(current_pair.start + current_pair.job->get_time_to_spend(), index));
    }
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
