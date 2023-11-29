#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    /*Job* first_job = new Job(0, 1, 1);
    Job* second_job = new Job(0, 2, 7);
    Job* third_job = new Job(0, 3, 2);
    Job* fourth_job = new Job(0, 4, 8);
    Job* fifth_job = new Job(0, 5, 8);
    Job* sixth_job = new Job(0, 6, 8);
    Job* seventh_job = new Job(0, 7, 8);
    all_jobs.push_back(first_job);
    all_jobs.push_back(second_job);
    all_jobs.push_back(third_job);
    all_jobs.push_back(fourth_job);
    all_jobs.push_back(fifth_job);
    all_jobs.push_back(sixth_job);
    all_jobs.push_back(seventh_job);
    first_job->set_predecessors({fourth_job, fifth_job});

    Plan common_plan = Plan({{8, 2}});
    Worker* first_worker = new Worker(common_plan);
    Worker* second_worker = new Worker(common_plan);
    all_workers.push_back(first_worker);
    all_workers.push_back(second_worker);
    WorkerGroup first_worker_group = WorkerGroup();
    first_worker_group.add_worker(first_worker);
    first_worker_group.add_worker(second_worker);



    int start_first_job_group_at = 1;
    JobGroup* first_job_group = new JobGroup({first_job, second_job, seventh_job}, start_first_job_group_at, 50);
    JobGroup* second_job_group = new JobGroup({third_job, fourth_job, fifth_job, sixth_job}, start_first_job_group_at, 200);
    algorithm.add_job_group(first_job_group, &first_worker_group);
    algorithm.add_job_group(second_job_group, &first_worker_group);
    algorithm.set_preference(current_preference);
    algorithm.run();*/
    ui->setupUi(this);
    setupPlot(ui->plot);
    //updatePlot(ui->plot, 15);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::GenerateExample()
{
    const int LOWEST_JOB_TIME = 3;
    const int HIGHEST_JOB_TIME = 20;
    const int ALL_JOBS_SIZE = 20000;
    const int ALL_WORKERS_SIZE = 500;
    const int JOB_GROUP_LOWEST_BEGIN = 0;
    const int JOB_GROUP_HIGHEST_BEGIN = 2;
    const int JOB_GROUP_LOWEST_END  = 100000;
    const int JOB_GROUP_HIGHEST_END = 10000000;
    const int JOB_GROUPS_COUNT = 100;
    qDebug() << "Begin example generation";
    all_jobs.clear();
    for (int i = 0; i < ALL_JOBS_SIZE; i++)
    {
        int time = QRandomGenerator::global()->bounded(LOWEST_JOB_TIME, HIGHEST_JOB_TIME);
        Job* generated = new Job(0, 0, time);
        all_jobs.push_back(generated);
        int predecessor = QRandomGenerator::global()->bounded(0, ALL_JOBS_SIZE);
        if (predecessor < i)
        {
            generated->set_predecessors({all_jobs[predecessor]});
        }
    }
    auto rng = std::default_random_engine {};
    std::shuffle(all_jobs.begin(), all_jobs.end(), rng);
    int current_job = 0;
    std::vector<JobGroup*> job_groups = std::vector<JobGroup*>();
    int true_job_groups_count = 0;
    for (int i = 0; i < JOB_GROUPS_COUNT; i++)
    {
        int new_group_size = QRandomGenerator::global()->bounded(ALL_JOBS_SIZE / JOB_GROUPS_COUNT - 20, ALL_JOBS_SIZE / JOB_GROUPS_COUNT + 20);
        if (current_job + new_group_size >= ALL_JOBS_SIZE || i == JOB_GROUPS_COUNT - 1)
        {
            new_group_size = ALL_JOBS_SIZE - current_job;
        }
        std::vector<Job*> new_group_jobs = std::vector<Job*>(new_group_size);
        for (int j = current_job; j < new_group_size + current_job; j++)
        {
            new_group_jobs[j - current_job] = all_jobs[j];
        }
        int begin = QRandomGenerator::global()->bounded(JOB_GROUP_LOWEST_BEGIN, JOB_GROUP_HIGHEST_BEGIN);
        int end = QRandomGenerator::global()->bounded(JOB_GROUP_LOWEST_END, JOB_GROUP_HIGHEST_END);
        job_groups.push_back(new JobGroup(new_group_jobs, begin, end));
        current_job += new_group_size;
        if (current_job >= ALL_JOBS_SIZE) break;
    }

    all_workers.clear();
    WorkerGroup* worker_group = new WorkerGroup();
    Plan common_plan = Plan({{30, 2}});
    for (int i = 0; i < ALL_WORKERS_SIZE; i++)
    {
        Worker* generated = new Worker(common_plan);
        all_workers.push_back(generated);
        worker_group->add_worker(generated);
    }

    for (int i = 0; i < job_groups.size(); i++)
    {
        algorithm.add_job_group(job_groups[i], worker_group);
    }

    std::shuffle(all_jobs.begin(), all_jobs.end(), rng);
    algorithm.set_preference(NONE);
    qDebug() << "example generated";
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
        auto found_worker = std::find(all_workers.begin(), all_workers.end(), current_pair.worker);
        int index_worker = std::distance(all_workers.begin(), found_worker);
        QCPItemRect* rect = new QCPItemRect( customPlot );
        rect->topLeft->setCoords(QPointF(current_pair.start, index + 1));
        switch (index_worker % 3)
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

    QCPItemRect* rect = new QCPItemRect( ui->plot );
    rect->topLeft->setCoords(QPointF(0,100));
    rect->bottomRight->setCoords(QPointF(100,0));
    customPlot->xAxis->setLabel("time");
    customPlot->yAxis->setLabel("tasks");
}

void MainWindow::on_spinBox_4_valueChanged(int arg1)
{
    start_first_job_group_at = arg1;
}

void MainWindow::on_spinBox_5_valueChanged(int arg1)
{
    current_preference = (Preference)arg1;
}

void MainWindow::on_pushButton_clicked()
{
    GenerateExample();
    algorithm.run();
    updatePlot(ui->plot, 10);
    /*for (int i = 0; i < all_jobs.size(); i++)
    {
        all_jobs[i]->undone();
    }
    WorkerGroup worker_group = WorkerGroup();
    for (int i = 0; i < all_workers.size(); i++)
    {
        int temp = __INT_MAX__;
        all_workers[i]->set_clock(&temp);
        all_workers[i]->update();
        worker_group.add_worker(all_workers[i]);
    }

    JobGroup* first_job_group = new JobGroup(all_jobs, start_first_job_group_at, __INT_MAX__);
    algorithm = Algorithm();
    algorithm.add_job_group(first_job_group, &worker_group);
    algorithm.set_preference(current_preference);
    algorithm.run();
    updatePlot(ui->plot, 10);*/
}
