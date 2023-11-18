#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "job.h"
#include "qcustomplot.h"
#include "colorlabel.h"
#include <qmatrix.h>
#include <QRandomGenerator>
#include <algorithm>
#include "algorithm.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void setupPlot(QCustomPlot *customPlot);
    void updatePlot(QCustomPlot *customPlot, int overall_time);
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_spinBox_4_valueChanged(int arg1);

    void on_spinBox_5_valueChanged(int arg1);

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    Algorithm algorithm;
    Preference current_preference = NONE;
    int start_first_job_group_at = 0;
    std::vector<Job*> all_jobs;
    std::vector<Worker*> all_workers;
};

#endif // MAINWINDOW_H
