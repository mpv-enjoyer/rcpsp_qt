#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Job* first = new Job(3, 0, 4);
    unassigned_jobs.push_back(first);
    assignJob(0, 2);

    Job* second = new Job(2, 0, 7);
    unassigned_jobs.push_back(second);
    assignJob(0, 0);

    Job* third = new Job(5, 0, 2);
    unassigned_jobs.push_back(third);
    assignJob(0, 5);

    ui->setupUi(this);

    setupPlot(ui->plot);
    updatePlot(ui->plot, 10);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updatePlot(QCustomPlot *customPlot, int overall_time)
{
    customPlot->clearItems();
    customPlot->addGraph();
    QVector<double> x(overall_time + 1), y(overall_time + 1);
    std::vector<int> drawn;
    for (int i = 0; i < overall_time; i++)
    {
        int current = 0;
        for (int j = 0; j < visual_jobs.size(); j++)
        {
            int begin = visual_jobs[j]->get_begin_at();
            int end = begin + visual_jobs[j]->get_time_to_spend();
            if (begin <= i && i < end)
            {
                if (std::find(drawn.begin(), drawn.end(), j) == drawn.end())
                {
                    drawn.push_back(j);
                }
                QCPItemRect* rect=new QCPItemRect( ui->plot );
                rect->topLeft->setCoords(QPointF(i,current));
                rect->setBrush(QBrush(visual_jobs[j]->get_color()));
                current += visual_jobs[j]->get_want_renewable();
                rect->bottomRight->setCoords(QPointF(i+1,current));
            }
        }
        x[i] = i;
        y[i] = current;
    }
    //customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));
    //customPlot->graph(0)->setData(x, y);
    customPlot->xAxis->setLabel("time");
    customPlot->yAxis->setLabel("renew");
    customPlot->xAxis->setRange(0, overall_time);
    customPlot->yAxis->setRange(0, renewables);
    for (int i = 0; i < previous_labels.size(); i++)
    {
        if (std::find(drawn.begin(), drawn.end(), previous_labels[i]->get_tag()) == drawn.end())
        {
            delete previous_labels[i];
            previous_labels.erase(previous_labels.begin() + i);
            continue;
        }
    }
    std::vector<int> previous_tags;
    for (int i = 0; i < previous_labels.size(); i++)
    {
        previous_tags.push_back(previous_labels[i]->get_tag());
    }
    for (int i = 0; i < drawn.size(); i++)
    {
        if (std::find(previous_tags.begin(), previous_tags.end(), drawn[i]) == previous_tags.end())
        {
            QPalette label_palette;
            //label_palette.setColor(QPalette::WindowText, Qt::black);
            label_palette.setColor(QPalette::WindowText, visual_jobs[drawn[i]]->get_color());
            ColorLabel* new_label = new ColorLabel(drawn[i]);
            ui->verticalLayout->addWidget(new_label);
            previous_labels.push_back(new_label);
            new_label->setPalette(label_palette);
            new_label->setText(QString::number(drawn[i]) + " job is assigned");
        }
    }
}

//Thanks to https://www.qcustomplot.com/index.php/support/forum/2213
void MainWindow::setupPlot(QCustomPlot *customPlot)
{
    customPlot->setInteraction(QCP::iRangeDrag, true);
    customPlot->setInteraction(QCP::iRangeZoom, true);

    QCPItemRect* rect=new QCPItemRect( ui->plot );
    rect->topLeft->setCoords(QPointF(0,100));
    rect->bottomRight->setCoords(QPointF(100,0));
    rect->setBrush(QBrush(QColor("floralwhite")));
}

void MainWindow::assignJob(int index, int time)
{
    AssignedJob* assigned = new AssignedJob(*(unassigned_jobs[index]), time);
    visual_jobs.push_back(new VisualJob(*assigned));
    delete unassigned_jobs[index];
    unassigned_jobs.erase(unassigned_jobs.begin() + index);
}
