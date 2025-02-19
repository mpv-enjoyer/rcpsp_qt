#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

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
    if (file_name.isEmpty())
    {
        Loader::Load("../sample.csv", algorithm, all_workers, all_jobs);
        Loader::LoadPreferences("../preferences.csv", algorithm);
        Loader::LoadWeights("../weights.csv", algorithm);
    }
    else
    {
        Loader::Load(file_name, algorithm, all_workers, all_jobs);
        algorithm.set_weights(Weights::create_equal());
    }
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

void MainWindow::on_actionOpen_triggered()
{
    file_name = QFileDialog::getOpenFileName(this, tr("Open CSV"), QDir::currentPath(), tr("Data Files (*.csv)"));
    ui->label->setText(file_name);
}

// Нужно собрать статистику: (может даже показать эти данные графиком в отдельной вкладке)
// 1) Какой процент работ выполняется сразу после того как поступает?
// 2) Какой процент в среднем заняло выполнение работы от того, сколько времени ей было выделено?