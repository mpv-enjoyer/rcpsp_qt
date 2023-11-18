#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "job.h"
#include "qcustomplot.h"
#include "colorlabel.h"
#include <qmatrix.h>
#include <QRandomGenerator>
#include <algorithm>

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
    void assignJob(int index, int time);

private:
    int renewables = 5;
    std::vector<Job*> unassigned_jobs;
    std::vector<VisualJob*> visual_jobs;
    Ui::MainWindow *ui;
    std::vector<ColorLabel*> previous_labels;
};

#endif // MAINWINDOW_H
