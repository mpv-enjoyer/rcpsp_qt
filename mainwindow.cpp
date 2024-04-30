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
    Loader::Load("input.csv", algorithm, all_workers, all_jobs);
    algorithm.run();
    _plot.updatePlot(algorithm.get_completed());
}

void MainWindow::on_pushButton_2_clicked()
{
    Generator generator("generated.csv", Plan({{100, 2}}));
    generator.lowestJobTime = 10;
    generator.highestJobTime = 20;

    generator.allJobsSize = 4000;
    generator.allWorkersSize = 25;

    generator.jobGroupLowestBegin = 0;
    generator.jobGroupHighestBegin = 1000;
    generator.jobGroupLowestEnd  = 1000;
    generator.jobGroupHighestEnd = 1100;

    generator.jobGroupsCount = 1;
    generator.groupSizeEntropy = 1;

    generator.generate_and_write();

    algorithm.run();
}
