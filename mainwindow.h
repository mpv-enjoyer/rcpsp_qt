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

private:
    Ui::MainWindow *ui;
    Algorithm algorithm;
    std::vector<Job*> all_jobs;
};

#endif // MAINWINDOW_H
