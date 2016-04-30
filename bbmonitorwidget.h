#ifndef BBMONITORWIDGET_H
#define BBMONITORWIDGET_H

#include <QWidget>
#include <qcustomplot.h>
#include <xwiimote.h>

#define PLOT_LENGTH 1000
#define PLOT_MAX_WEIGHT 8000

namespace Ui {
class BBMonitorWidget;
}

class BBMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BBMonitorWidget(QWidget *parent = 0);
    ~BBMonitorWidget();

public slots:
    void readReady();

private:
    Ui::BBMonitorWidget *ui;
    void loadPlot();
    void setupQueues();
    void setupSocketNotifier();
    void handle_bboard_event(const struct xwii_event *event);
    void setup_csv_file();
    QVector<double> x;
    QQueue<double> queue_w;
    QQueue<double> queue_x;
    QQueue<double> queue_y;
    QQueue<double> queue_z;
    QQueue<double> queue_sum;
    QVector<double> center_of_mass_x;
    QVector<double> center_of_mass_y;
    QSocketNotifier* sn;
    struct xwii_iface *iface;
    QFile csv_file;
};

#endif // BBMONITORWIDGET_H
