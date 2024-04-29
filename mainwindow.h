#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QString>
#include "job.h"
#include "qcustomplot.h"
#include <qmatrix.h>
#include <algorithm>
#include "algorithm.h"
#include <signal.h>
#include "chartview.h"
#include "generator.h"
#include "plot.h"
#include "loader.h"

#undef QT_NO_DEBUG_OUTPUT

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    //void setupPlot(QCustomPlot *customPlot, const std::vector<ResultPair>& current_completed);
    void on_pushButton_2_clicked();

private:
    Plot _plot;
    std::vector<int> workers_indexes;
    int start_first_job_group_at = 0;
    Preference current_preference = NONE;
    Ui::MainWindow *ui;
    Algorithm algorithm;
    std::vector<Job*> all_jobs;
    std::vector<Worker*> all_workers;
    void LoadCSV(QString file_name);
    ChartView* chartview;
};

#endif // MAINWINDOW_H
