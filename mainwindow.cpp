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
    AlgorithmWeights w = Loader::LoadWeights("weights.csv");
    auto niter = Weights::WeightsNames.begin();
    ui->doubleSpinBox_2->setValue(Weights::get(w, *(niter++)));
    ui->doubleSpinBox_3->setValue(Weights::get(w, *(niter++)));
    ui->doubleSpinBox_4->setValue(Weights::get(w, *(niter++)));
    ui->doubleSpinBox_5->setValue(Weights::get(w, *(niter++)));
    ui->doubleSpinBox_6->setValue(Weights::get(w, *(niter++)));
    ui->doubleSpinBox_7->setValue(Weights::get(w, *(niter++)));
    ui->doubleSpinBox_8->setValue(Weights::get(w, *(niter)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

enum COMBOBOX_INDEXES
{
    COMBOBOX_INDEX_NONE = 0,
    COMBOBOX_INDEX_LPT = 1,
    COMBOBOX_INDEX_SPT = 2,
    COMBOBOX_INDEX_SLS = 3,
    COMBOBOX_INDEX_LWS = 4
};

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
    if (file_name.isEmpty())
    {
        return; /* ignore */
        Loader::Load("../sample.csv", algorithm);
        Loader::LoadPreferences("../preferences.csv", algorithm);
        Loader::LoadWeights("../weights.csv", algorithm);
    }
    else
    {
        AlgorithmWeights ws = Weights::create_equal(); // ONLY FOR LWS
        auto niter = Weights::WeightsNames.begin(); // ONLY FOR LWS
        Loader::Load(file_name, algorithm);
        Loader::LoadPreferences(file_name, algorithm);
        Loader::LoadWeights(file_name, algorithm);
        auto wwws = algorithm.get_weights(); // ONLY FOR NONE
        switch (ui->comboBox->currentIndex()) {
        case COMBOBOX_INDEX_NONE:
            if (!Weights::are_valid(wwws))
            {
                algorithm.set_weights(Weights::create_empty());
            }
            else
            {
                algorithm.set_weights(wwws);
            }
            break;
        case COMBOBOX_INDEX_LPT: algorithm.set_preference(LPT); break;
        case COMBOBOX_INDEX_SPT: algorithm.set_preference(SPT); break;
        case COMBOBOX_INDEX_SLS: algorithm.set_preference(FLS); break;
        case COMBOBOX_INDEX_LWS:
            Weights::set(ws, *(niter++), ui->doubleSpinBox_2->value());
            Weights::set(ws, *(niter++), ui->doubleSpinBox_3->value());
            Weights::set(ws, *(niter++), ui->doubleSpinBox_4->value());
            Weights::set(ws, *(niter++), ui->doubleSpinBox_5->value());
            Weights::set(ws, *(niter++), ui->doubleSpinBox_6->value());
            Weights::set(ws, *(niter++), ui->doubleSpinBox_7->value());
            Weights::set(ws, *(niter), ui->doubleSpinBox_8->value());
            assert(Weights::are_valid(ws));
            algorithm.set_weights(ws);
            algorithm.set_preference(NONE);
            break;
        default:
            break;
        }
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
        QString("Lead to impossible jobs: ") +
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
        QString("%), Penalty: ") +
        QString::number(algorithm.get_penalty()));
    QApplication::alert(this);
}

void MainWindow::on_actionOpen_triggered()
{
    file_name = QFileDialog::getOpenFileName(this, tr("Open CSV"), QDir::currentPath(), tr("Data Files (*.csv)"));
    ui->label->setText(file_name);
}

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

#include <QDesktopServices>

void MainWindow::on_button_current_file_clicked()
{
    if (this->file_name.isEmpty()) return;
    QDesktopServices::openUrl(QUrl(QString("file://") + this->file_name));
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    bool is_enabled = index == COMBOBOX_INDEX_LWS;
    ui->doubleSpinBox_2->setEnabled(is_enabled);
    ui->doubleSpinBox_3->setEnabled(is_enabled);
    ui->doubleSpinBox_4->setEnabled(is_enabled);
    ui->doubleSpinBox_5->setEnabled(is_enabled);
    ui->doubleSpinBox_6->setEnabled(is_enabled);
    ui->doubleSpinBox_7->setEnabled(is_enabled);
    ui->doubleSpinBox_8->setEnabled(is_enabled);
}

void MainWindow::on_actionCopy_weights_from_clipboard_triggered()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString string = clipboard->text();
    QStringList weights_strs = string.split(',');
    if (weights_strs.size() != Weights::SIZE) return;
    QStringList::iterator iter = weights_strs.begin();
    ui->doubleSpinBox_2->setValue((iter++)->toFloat(nullptr));
    ui->doubleSpinBox_3->setValue((iter++)->toFloat(nullptr));
    ui->doubleSpinBox_4->setValue((iter++)->toFloat(nullptr));
    ui->doubleSpinBox_5->setValue((iter++)->toFloat(nullptr));
    ui->doubleSpinBox_6->setValue((iter++)->toFloat(nullptr));
    ui->doubleSpinBox_7->setValue((iter++)->toFloat(nullptr));
    ui->doubleSpinBox_8->setValue((iter)->toFloat(nullptr));
    ui->comboBox->setCurrentIndex(COMBOBOX_INDEX_LWS);
}


void MainWindow::on_actionCopy_legacy_weights_triggered()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString string = clipboard->text();
    QStringList weights_strs = string.split(',');
    if (weights_strs.size() != Weights::SIZE) return;
    QStringList::iterator iter = weights_strs.begin();
    ui->doubleSpinBox_2->setValue(-(iter++)->toFloat(nullptr));
    ui->doubleSpinBox_3->setValue(-(iter++)->toFloat(nullptr));
    ui->doubleSpinBox_4->setValue(-(iter++)->toFloat(nullptr));
    ui->doubleSpinBox_5->setValue(-(iter++)->toFloat(nullptr));
    ui->doubleSpinBox_6->setValue(-(iter++)->toFloat(nullptr));
    ui->doubleSpinBox_7->setValue(-(iter++)->toFloat(nullptr));
    ui->doubleSpinBox_8->setValue(-(iter)->toFloat(nullptr));
    ui->comboBox->setCurrentIndex(COMBOBOX_INDEX_LWS);
}

