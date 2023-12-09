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
    first_job->set_ancestors({fourth_job, fifth_job});

    Plan common_plan = Plan({{8, 2}});
    Worker* first_worker = new Worker(common_plan);
    Worker* second_worker = new Worker(common_plan);
    Worker* third_worker = new Worker(common_plan);
    all_workers.push_back(first_worker);
    all_workers.push_back(second_worker);
    all_workers.push_back(third_worker);
    WorkerGroup first_worker_group = WorkerGroup();
    WorkerGroup second_worker_group = WorkerGroup();
    first_worker_group.add_worker(first_worker);
    first_worker_group.add_worker(second_worker);
    second_worker_group.add_worker(third_worker);

    int start_first_job_group_at = 1;
    JobGroup* first_job_group = new JobGroup({first_job, second_job, seventh_job}, start_first_job_group_at, 50);
    JobGroup* second_job_group = new JobGroup({third_job, fourth_job, fifth_job, sixth_job}, start_first_job_group_at, 200);
    algorithm.add_job_group(first_job_group, &second_worker_group);
    algorithm.add_job_group(second_job_group, &first_worker_group);
    algorithm.set_preference(EST);
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
    const int ALL_JOBS_SIZE = 10000;
    const int ALL_WORKERS_SIZE = 500;
    const int JOB_GROUP_LOWEST_BEGIN = 0;
    const int JOB_GROUP_HIGHEST_BEGIN = 1000;
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
            generated->set_ancestors({all_jobs[predecessor]});
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
    //workers_indexes = std::vector<int>(all_jobs.size(), -1);
    model = new ResultModel();
    for (int i = 0; i < current_completed.size(); i++)
    {
        ResultPair current_pair = current_completed[i];
        //auto found = std::find(all_jobs.begin(), all_jobs.end(), current_pair.job);
        //int index = std::distance(all_jobs.begin(), found);
        int index = current_pair.job_id;
        //auto found_worker = std::find(all_workers.begin(), all_workers.end(), current_pair.worker);
        //int index_worker = std::distance(all_workers.begin(), found_worker);
        //workers_indexes[index] = index_worker;
        QCPItemRect* rect = new QCPItemRect( customPlot );
        rect->setBrush(QBrush(QColor("yellow")));
        rect->topLeft->setCoords(QPointF(current_pair.start, - index - 1));
        rect->bottomRight->setCoords(QPointF(current_pair.start + current_pair.job->get_time_to_spend(), - index));
        model->append(current_pair);
    }
    ui->tableView->setModel(model);
    int top_column = ui->tableView->indexAt(ui->tableView->rect().topLeft()).row();
    int bottom_column = ui->tableView->indexAt(ui->tableView->rect().bottomLeft()).row();
    customPlot->yAxis->setRange(top_column, bottom_column, Qt::AlignCenter);
    //customPlot->setViewport(QRect(0, bottom_column, 100, top_column));
    //customPlot->setFixedHeight(bottom_column - top_column);
    customPlot->replot();
    customPlot->rescaleAxes();
    emit MainWindow::yAxisChanged(QCPRange(top_column, bottom_column));
    //ui->tableView->scrollTo(index);
    //model->sort(0);
}

//Thanks to https://www.qcustomplot.com/index.php/support/forum/2213
void MainWindow::setupPlot(QCustomPlot *customPlot)
{
    customPlot->setInteraction(QCP::iRangeDrag, true);
    customPlot->setInteraction(QCP::iRangeZoom, true);
    //customPlot->setInteraction(QCP::iSelectItems, true);
    //customPlot->setInteraction(QCP::);
    customPlot->xAxis2->setLabel("time");
    customPlot->yAxis->setTicks(false);
    connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(yAxisChanged(QCPRange)));
    //connect(ui->tableView->verticalScrollBar(), signal(valueChanged(int)));//, this, SLOT(yAxisChanged(QCPRange)))
    // initialize axis range (and scroll bar positions via signals we just connected):
    customPlot->xAxis2->setRange(0, 10, Qt::AlignLeft);
    customPlot->yAxis->setRange(0, 10, Qt::AlignLeft);
    //rightTag->setPen(mGraph1->pen());
    //connect(customPlot, SIGNAL(itemClick(QCPAbstractItem*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractItem*,QMouseEvent*)));
    //connect(customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(graphClicked()));
    QHeaderView* header=ui->tableView->verticalHeader();
    header->setDefaultSectionSize(20); // 20 px height
    header->sectionResizeMode(QHeaderView::Fixed);
    //customPlot->axisRect(0)->addAxis(QCPAxis::atTop);
}

/*void MainWindow::graphClicked(QCPAbstractItem* item, QMouseEvent* mouse_event)
{
    //QCPSelectionRect* current_rect = ui->plot->selectionRect();
    int left = item->clipAxisRect()->left();
    int bottom = item->clipAxisRect()->top();
    if (bottom == -1) return;
    //if (workers_indexes[current_rect->rect().bottom()] == -1) return;
    QString message = QString("Job '%1' Worker #%2 at %3 tick.").arg(bottom).arg(workers_indexes[bottom]).arg(left);
    ui->textBrowser->append(message);
    // since we know we only have QCPGraphs in the plot, we can immediately access interface1D()
    // usually it's better to first check whether interface1D() returns non-zero, and only then use it.
    //double dataValue = plottable->interface1D()->dataMainValue(dataIndex);
    //QString message = QString("Clicked on graph '%1' at data point #%2 with value %3.").arg(plottable->name()).arg(dataIndex).arg(dataValue);
    //ui->textBrowser->append(message);
    //ui->statusbar->showMessage(message, 2500);
}*/

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
    LoadCSV("input.csv");
    static bool used = false;
    if (used) return;
    //GenerateExample();
    algorithm.run();
    updatePlot(ui->plot, 10);
    used = true;
}

void MainWindow::yAxisChanged(QCPRange range)
{
    //qDebug() << ui->plot->move();
    //ui->plot->yAxis->setRange(range.lower, range.upper, Qt::AlignCenter);
    //ui->tableView->showRow(range.lower);
    range.lower *= -1;
    range.upper *= -1;
    if (!ui->tableView->model()) return;
    auto index = ui->tableView->model()->index(range.lower, 0);
    auto index2 = ui->tableView->model()->index(range.upper, 0);
    ui->tableView->scrollTo(index);
    ui->tableView->scrollTo(index2);
    QHeaderView* header=ui->tableView->verticalHeader();
    int bottom_child = ui->tableView->childrenRect().bottom();
    int upper_child = ui->tableView->childrenRect().top() + 41;
    header->setDefaultSectionSize((bottom_child - upper_child) / (range.lower - range.upper));

    //header->sectionResizeMode(QHeaderView::Fixed);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{

}

void MainWindow::LoadCSV(QString file_name)
{
    QFile file(file_name);
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) {
        qDebug() << "File not exists";
        return;
    }
    // Создаём поток для извлечения данных из файла
    QTextStream in(&file);
    // Считываем данные до конца файла
    std::vector<JobLoad> jobs_load;
    std::vector<WorkerLoad> workers_load;
    std::vector<PlanLoad> plans_load;
    std::vector<JobGroupLoad> job_groups_load;
    std::vector<WorkerGroupLoad> worker_groups_load;
    algorithm.set_preference(NONE);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list = line.split(";");
        if (list[0] == "job")
        {
            JobLoad current = { nullptr, 0, 0, std::vector<int>() };
            current.id = list[1].toInt();
            current.time = list[2].toInt();
            for (int i = 3; i < list.size(); i++)
            {
                current.ancestors.push_back(list[i].toInt());
            }
            jobs_load.push_back(current);
        }
        if (list[0] == "worker")
        {
            WorkerLoad current = { nullptr, 0, 0 };
            current.id = list[1].toInt();
            current.plan = list[2].toInt();
            workers_load.push_back(current);
        }
        if (list[0] == "plan")
        {
            PlanLoad current = { nullptr, 0, 0, std::vector<PlanElement>() };
            current.id = list[1].toInt();
            current.start_at = list[2].toInt();
            for (int i = 3; i < list.size(); i+=2)
            {
                current.plan.push_back({ list[i].toInt(), list[i + 1].toInt() });
            }
            plans_load.push_back(current);
        }
        if (list[0] == "job_group")
        {
            JobGroupLoad current = { nullptr, 0, 0, 0, 0, std::vector<int>() };
            current.id = list[1].toInt();
            current.start_after = list[2].toInt();
            current.end_before = list[3].toInt();
            current.worker_group = list[4].toInt();
            for (int i = 5; i < list.size(); i++)
            {
                current.jobs.push_back(list[i].toInt());
            }
            job_groups_load.push_back(current);
        }
        if (list[0] == "worker_group")
        {
            WorkerGroupLoad current = { nullptr, 0, std::vector<int>() };
            current.id = list[1].toInt();
            for (int i = 2; i < list.size(); i++)
            {
                current.workers.push_back(list[i].toInt());
            }
            worker_groups_load.push_back(current);
        }
        if (list[0] == "preference")
        {
            if (list[1] == "SPT") algorithm.set_preference(SPT);
            if (list[1] == "LPT") algorithm.set_preference(LPT);
            if (list[1] == "EST") algorithm.set_preference(EST);
        }
        // 1) job | worker | plan | job_group | worker_group | preference
        // 2) id (from 0 without skips) | preference: "SPT | LPT | EST"
        // 3) job: time | worker: plan_id | plan: start_at | job_group: start_after | worker_group: workers...
        // 4) job: ancestors... | worker: | plan: work, rest... | job_group: end_before |
        // 5) | | | job_group: worker_group
        // 6) | | | job_group: jobs...
    }
    file.close();

    for (int i = 0; i < plans_load.size(); i++)
    {
        plans_load[i].assign = new Plan(plans_load[i].plan, plans_load[i].start_at);
        std::swap(plans_load[i], plans_load[plans_load[i].id]);
    }
    all_workers = std::vector<Worker*>(workers_load.size());
    for (int i = 0; i < workers_load.size(); i++)
    {
        workers_load[i].assign = new Worker(*plans_load[workers_load[i].plan].assign);
        int id_true = workers_load[i].id;
        std::swap(workers_load[i], workers_load[id_true]);
        all_workers[id_true] = workers_load[id_true].assign;
    }
    for (int i = 0; i < worker_groups_load.size(); i++)
    {
        WorkerGroup* worker_group = new WorkerGroup();
        worker_groups_load[i].assign = worker_group;
        std::swap(worker_groups_load[i], worker_groups_load[worker_groups_load[i].id]);
        for (int j = 0; j < worker_groups_load[i].workers.size(); j++)
        {
            worker_group->add_worker(workers_load[worker_groups_load[i].workers[j]].assign);
        }
    }
    all_jobs = std::vector<Job*>(jobs_load.size());
    for (int i = 0; i < jobs_load.size(); i++)
    {
        std::swap(jobs_load[i], jobs_load[jobs_load[i].id]);
    }
    bool changed = true;
    while (changed)
    {
        changed = false;
        for (int i = 0; i < jobs_load.size(); i++)
        {
            if (jobs_load[i].assign) continue;
            bool all_ancestors_assigned = true;
            std::vector<Job*> ancestors;
            for (int j = 0; j < jobs_load[i].ancestors.size(); j++)
            {
                if (!jobs_load[jobs_load[i].ancestors[j]].assign)
                {
                    all_ancestors_assigned = false;
                    break;
                }
                ancestors.push_back(jobs_load[jobs_load[i].ancestors[j]].assign);
            }
            if (!all_ancestors_assigned) continue;
            changed = true;
            Job* job = new Job(0, 0, jobs_load[i].time);
            job->set_ancestors(ancestors);
            jobs_load[i].assign = job;
            all_jobs[i] = job;
        }
    }
    for (int i = 0; i < job_groups_load.size(); i++)
    {
        std::swap(job_groups_load[i], job_groups_load[job_groups_load[i].id]);
    }
    for (int i = 0; i < job_groups_load.size(); i++)
    {
        std::vector<Job*> want_jobs;
        for (int j = 0; j < job_groups_load[i].jobs.size(); j++)
        {
            int id = job_groups_load[i].jobs[j];
            want_jobs.push_back(jobs_load[id].assign);
        }
        job_groups_load[i].assign = new JobGroup(want_jobs, job_groups_load[i].start_after, job_groups_load[i].end_before);
        int current_worker_group_id = job_groups_load[i].worker_group;
        algorithm.add_job_group(job_groups_load[i].assign, worker_groups_load[current_worker_group_id].assign);
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    //if ( object == ui->tableView && ( event->type() == QEvent::Scroll ) ) {
        //event->Scroll
      //  ui->tableView->showRow(1);
    //}
    return true;
}
