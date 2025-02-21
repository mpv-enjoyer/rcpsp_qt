#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _plot = Plot(ui->plot);
    _wait_stats_plot = WaitStatsPlot(ui->wait_coeff_plot);
    _work_stats_plot = WorkStatsPlot(ui->work_coeff_plot);
    //
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if (!(ui->checkbox_logs->isChecked()))
    {
        qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) { });
    }
    else
    {
        qInstallMessageHandler(0);
    }
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
    auto completed = algorithm.get_completed();
    _plot.updatePlot(completed);
    Stats stats = Stats(completed, ui->doubleSpinBox->value(), true);
    _work_stats_plot.updatePlot(stats);
    _wait_stats_plot.updatePlot(stats);
    double impossible_percent = stats.leads_to_impossible_jobs_counter / double(completed.size()) * 100;
    double good_percent = (completed.size() - algorithm.get_failed_jobs_count()) / double(completed.size()) * 100;
    ui->label_counter_impossible_jobs->setText(
        QString("Leads to impossible jobs: ") +
        QString::number(stats.leads_to_impossible_jobs_counter) +
        QString(" / ") +
        QString::number(completed.size()) +
        QString(" (") +
        QString::number(impossible_percent) +
        QString("%). Good: ") +
        QString::number(completed.size() - algorithm.get_failed_jobs_count()) +
        QString(" / ") +
        QString::number(completed.size()) +
        QString(" (") +
        QString::number(good_percent) +
        QString("%)"));
}

void MainWindow::on_actionOpen_triggered()
{
    file_name = QFileDialog::getOpenFileName(this, tr("Open CSV"), QDir::currentPath(), tr("Data Files (*.csv)"));
    ui->label->setText(file_name);
}

// Нужно собрать статистику: (может даже показать эти данные графиком в отдельной вкладке)
// 1) Какой процент работ выполняется сразу после того как поступает?
// 2) Какой процент в среднем заняло выполнение работы от того, сколько времени ей было выделено?

void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{

}

void MainWindow::on_doubleSpinBox_editingFinished()
{
    constexpr static double MIN_VALUE = 0.00001;
    double value = ui->doubleSpinBox->value();
    if (value == 0)
    {
        ui->doubleSpinBox->setValue(MIN_VALUE);
        value = MIN_VALUE;
    }
    auto completed = algorithm.get_completed();
    Stats stats = Stats(completed, value);
    _work_stats_plot.updatePlot(stats);
    _wait_stats_plot.updatePlot(stats);
}

void MainWindow::on_checkbox_logs_checkStateChanged(const Qt::CheckState &arg1)
{
    //if ((!arg1) == Qt::CheckState::Checked) qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) { });
    //else if (arg1 == Qt::CheckState::Unchecked) qInstallMessageHandler(0);
}

#include <QClipboard>

void MainWindow::on_button_copy_logs_clicked()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    auto qstr = QString::fromStdString(algorithm.get_string_result(algorithm.get_completed()));
    clipboard->setText(qstr);
}
