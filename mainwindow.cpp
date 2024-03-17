#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updatePlot(int overall_time)
{
    std::vector<ResultPair> current_completed = algorithm.get_completed();
    setupPlot(ui->plot, current_completed);
}

QCPBars* createBars(QString name, QColor color, QCustomPlot* plot)
{
    QCPBars *set = new QCPBars(plot->yAxis, plot->xAxis);
    set->setAntialiased(false);
    set->setStackingGap(0);
    set->setName(name);
    set->setPen(QColor::fromRgb(0, 0, 0, 0));
    set->setBrush(color);
    return set;
}

void MainWindow::setupPlot(QCustomPlot *customPlot, const std::vector<ResultPair>& current_completed)
{
    // set dark background gradient:
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor(90, 90, 90));
    gradient.setColorAt(0.38, QColor(105, 105, 105));
    gradient.setColorAt(1, QColor(70, 70, 70));
    customPlot->setBackground(QBrush(gradient));

    // create empty bar chart objects:
    QCPBars *unable_set = createBars("", QColor::fromRgb(255, 255, 255, 0), customPlot);
    QCPBars *waiting_set = createBars("Поступление работы - Начало обслуживания", QColor::fromRgb(200, 200, 200), customPlot);
    QCPBars *executed_set = createBars("Начало обслуживания - Конец обслуживания", QColor::fromRgb(0, 0, 0), customPlot);
    QCPBars *critical_set = createBars("Конец обслуживания - Позднее время начала", QColor::fromRgb(200, 150, 150), customPlot);
    QCPBars *ready_set = createBars("Позднее время начала - Директивный срок", QColor::fromRgb(200, 200, 200), customPlot);
    // stack bars on top of each other:
    waiting_set->moveAbove(unable_set);
    executed_set->moveAbove(waiting_set);
    critical_set->moveAbove(executed_set);
    ready_set->moveAbove(critical_set);
    // prepare y axis with country labels:
    auto y_axis_size = current_completed.size();
    QVector<double> ticks(y_axis_size);
    QVector<QString> labels(y_axis_size);

    for (auto i = 0; i < y_axis_size; i++)
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
    }

    customPlot->xAxis->setTickLabelRotation(60);
    customPlot->xAxis->setSubTicks(false);
    customPlot->xAxis->setTickLength(0, 4);
    customPlot->xAxis->setRange(0, 8);
    customPlot->xAxis->setBasePen(QPen(Qt::white));
    customPlot->xAxis->setTickPen(QPen(Qt::white));
    customPlot->xAxis->grid()->setVisible(true);
    customPlot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis->setLabelColor(Qt::white);

    // prepare y axis:
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    customPlot->yAxis->setTicker(textTicker);
    customPlot->yAxis->setRange(0, 12.1);
    customPlot->yAxis->setPadding(5); // a bit more space to the left border
    customPlot->yAxis->setLabel("Gantt chart (QCustomPlot)");
    customPlot->yAxis->setBasePen(QPen(Qt::white));
    customPlot->yAxis->setTickPen(QPen(Qt::white));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white));
    customPlot->yAxis->grid()->setSubGridVisible(true);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setLabelColor(Qt::white);
    customPlot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));

    // Add data:

    QVector<double> start_after_data(y_axis_size);
    QVector<double> waiting_data(y_axis_size);
    QVector<double> in_progress_data(y_axis_size);
    QVector<double> critical_data(y_axis_size);
    QVector<double> ready_data(y_axis_size);
    for (auto i = 0; i < y_axis_size; i++)
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
    }
    unable_set->setData(ticks, start_after_data);
    waiting_set->setData(ticks, waiting_data);
    executed_set->setData(ticks, in_progress_data);
    critical_set->setData(ticks, critical_data);
    ready_set->setData(ticks, ready_data);

    // setup legend:
    customPlot->legend->setVisible(true);
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    customPlot->legend->setBrush(QColor(255, 255, 255, 100));
    customPlot->legend->setBorderPen(Qt::NoPen);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    customPlot->legend->setFont(legendFont);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    customPlot->replot();
}

void MainWindow::on_pushButton_clicked()
{
    LoadCSV("input.csv");
    static bool used = false;
    if (used) return;
    //GenerateExample();
    algorithm.run();
    updatePlot(10);
    used = true;
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
            if (list[1] == "FLS") algorithm.set_preference(EST);
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
            Job* job = new Job(0, 0, jobs_load[i].time, 0.5f);
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

void MainWindow::on_pushButton_2_clicked()
{
    Generator generator("generated.csv", Plan({{30, 2}}));
    generator.lowestJobTime = 10;
    generator.highestJobTime = 20;

    generator.allJobsSize = 100000;
    generator.allWorkersSize = 500;

    generator.jobGroupLowestBegin = 0;
    generator.jobGroupHighestBegin = 1000;
    generator.jobGroupLowestEnd  = 10000;
    generator.jobGroupHighestEnd = 10000000;

    generator.jobGroupsCount = 1;
    generator.groupSizeEntropy = 1;

    generator.generate_and_write();

    algorithm.run();
    updatePlot(10);
}
