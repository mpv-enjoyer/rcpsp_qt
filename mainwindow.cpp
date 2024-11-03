#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _plot = Plot(ui->plot);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    workers_indexes.clear();
    start_first_job_group_at = 0;
    algorithm = Algorithm();
    Loader::Load("../input.csv", algorithm, all_workers, all_jobs);
    Loader::LoadPreferences("../preferences.csv", algorithm);
    Loader::LoadWeights("../weights.csv", algorithm);
    algorithm.run();
    _plot.updatePlot(algorithm.get_completed());
}

void MainWindow::on_pushButton_2_clicked()
{
    Generator generator("generated.csv", Plan({{1000, 2}}));
    generator.lowestJobTime = 50;
    generator.highestJobTime = 70;

    generator.allJobsSize = 5000;
    generator.allWorkersSize = 20;

    // запретил частичную занятость

    generator.jobGroupLowestBegin = 0;
    generator.jobGroupHighestBegin = 1000;
    generator.jobGroupLowestEnd  = 10000;
    generator.jobGroupHighestEnd = 50000;

    generator.jobGroupsCount = 30;
    generator.groupSizeEntropy = 4;

    generator.look_ahead = 0;

    generator.generate_and_write();

    algorithm.run();
}
