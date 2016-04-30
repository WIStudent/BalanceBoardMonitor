#include "bbmonitorwidget.h"
#include "ui_bbmonitorwidget.h"

static int enumerate();

BBMonitorWidget::BBMonitorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BBMonitorWidget),
    csv_file("bboard_data.csv")
{
    ui->setupUi(this);
    setupQueues();
    setup_csv_file();
    loadPlot();
    enumerate();
    setupSocketNotifier();
}

BBMonitorWidget::~BBMonitorWidget()
{
    delete ui;
}

/**
 * @brief BBMonitorWidget::loadPlot
 *        Update plots with the newest data.
 *
 */
void BBMonitorWidget::loadPlot()
{
    // W plot
    QCustomPlot *customPlotW = ui->PlotWidgetW;
    // create graph and assign data to it:
    customPlotW->addGraph();
    customPlotW->graph(0)->setData(x, queue_w.toVector());
    // give the axes some labels:
    customPlotW->yAxis->setLabel("Weight W");
    // set axes ranges, so we see all data:
    customPlotW->xAxis->setRange(0, PLOT_LENGTH);
    customPlotW->yAxis->setRange(0, PLOT_MAX_WEIGHT);
    customPlotW->replot();

    // X plot
    QCustomPlot *customPlotX = ui->PlotWidgetX;
    // create graph and assign data to it:
    customPlotX->addGraph();
    customPlotX->graph(0)->setData(x, queue_x.toVector());
    // give the axes some labels:
    customPlotX->yAxis->setLabel("Weight X");
    // set axes ranges, so we see all data:
    customPlotX->xAxis->setRange(0, PLOT_LENGTH);
    customPlotX->yAxis->setRange(0, PLOT_MAX_WEIGHT);
    customPlotX->replot();

    // Y plot
    QCustomPlot *customPlotY = ui->PlotWidgetY;
    // create graph and assign data to it:
    customPlotY->addGraph();
    customPlotY->graph(0)->setData(x, queue_y.toVector());
    // give the axes some labels:
    customPlotY->yAxis->setLabel("Weight Y");
    // set axes ranges, so we see all data:
    customPlotY->xAxis->setRange(0, PLOT_LENGTH);
    customPlotY->yAxis->setRange(0, PLOT_MAX_WEIGHT);
    customPlotY->replot();

    // Z plot
    QCustomPlot *customPlotZ = ui->PlotWidgetZ;
    // create graph and assign data to it:
    customPlotZ->addGraph();
    customPlotZ->graph(0)->setData(x, queue_z.toVector());
    // give the axes some labels:
    customPlotZ->yAxis->setLabel("Weight Z");
    // set axes ranges, so we see all data:
    customPlotZ->xAxis->setRange(0, PLOT_LENGTH);
    customPlotZ->yAxis->setRange(0, PLOT_MAX_WEIGHT);
    customPlotZ->replot();

    // sum plot
    QCustomPlot *customPlotSum = ui->PlotWidgetSum;
    // create graph and assign data to it:
    customPlotSum->addGraph();
    customPlotSum->graph(0)->setData(x, queue_sum.toVector());
    // give the axes some labels:
    customPlotSum->yAxis->setLabel("Weight Sum");
    // set axes ranges, so we see all data:
    customPlotSum->xAxis->setRange(0, PLOT_LENGTH);
    customPlotSum->yAxis->setRange(0, PLOT_MAX_WEIGHT);
    customPlotSum->replot();

    // center of mass
    QCustomPlot *customPlotMassCenter = ui->PlotWidgetMassCenter;
    // create graph and assign data to it:
    customPlotMassCenter->addGraph();
    customPlotMassCenter->graph(0)->setData(center_of_mass_x, center_of_mass_y);
    // set scatter symbol
    customPlotMassCenter->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 10));
    // give the axes some labels:
    customPlotMassCenter->xAxis->setLabel("x");
    customPlotMassCenter->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    customPlotMassCenter->xAxis->setRange(-1, 1);
    customPlotMassCenter->yAxis->setRange(-1, 1);
    customPlotMassCenter->replot();
}

/**
 * @brief BBMonitorWidget::setupQueues
 *        Initialize the variables that hold the data for the queues.
 */
void BBMonitorWidget::setupQueues()
{
    x = QVector<double>(PLOT_LENGTH);
    for(int i = 0; i < PLOT_LENGTH; i++)
    {
        x[i] = i;
        queue_w.enqueue(0);
        queue_x.enqueue(0);
        queue_y.enqueue(0);
        queue_z.enqueue(0);
        queue_sum.enqueue(0);
    }
    center_of_mass_x = QVector<double>(1);
    center_of_mass_x[0] = 0;
    center_of_mass_y = QVector<double>(1);
    center_of_mass_y[0] = 0;
}

/**
 * @brief BBMonitorWidget::setup_csv_file
 *        Setup the csv file
 */
void BBMonitorWidget::setup_csv_file()
{
    if (csv_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&csv_file);
        stream << "time,v,x,y,z" << endl;
        csv_file.close();
    }
}

/**
 * @brief BBMonitorWidget::setupSocketNotifier
 *        Check if a Balance Board is connected and register a QSocketNotifier that
 *        calls readReady() as soon as the Balance Board's interface is readable.
 */
void BBMonitorWidget::setupSocketNotifier()
{

    int ret = 0;
    char *path = NULL;

    struct xwii_monitor *mon;
    int num = 0;

    mon = xwii_monitor_new(false, false);
    if (!mon)
    {
        qDebug("Cannot create monitor\n");
        exit(EXIT_FAILURE);
    }

    bool found_balance_board = false;
    while (!found_balance_board && (path = xwii_monitor_poll(mon)))
    {

        qDebug("  Found device #%d: %s\n", ++num, path);

        ret = xwii_iface_new(&iface, path);

        if (ret)
        {
            qDebug("Cannot create xwii_iface '%s' err:%d\n",path, ret);
            exit(EXIT_FAILURE);
        }
         free(path);

        char *deftype = NULL;
        xwii_iface_get_devtype(iface, &deftype);

        // Check if a balance board was found
        found_balance_board = (strcmp(deftype,"balanceboard") == 0);
        free(deftype);
        // if it was not a balance board, destroy the iface again
        if(!found_balance_board)
        {
            xwii_iface_unref(iface);
            iface = NULL;
        }

    }
    xwii_monitor_unref(mon);

    // Exit if no balance board was connected.
    if(!found_balance_board)
    {
        qDebug("No balance board connected!");
        free(path);
        exit(EXIT_FAILURE);
    }


    ret = xwii_iface_open(iface, xwii_iface_available(iface) | XWII_IFACE_WRITABLE);
    if (ret)
    {
        qDebug("Error: Cannot open interface: %d. Forgot sudo?\n", ret);
        exit(EXIT_FAILURE);
    }

    ret = xwii_iface_watch(iface, true);
    if (ret)
    {
        qDebug("Error: Cannot initialize hotplug watch descriptor\n");
    }

    // QSocketNotifier
    sn = new QSocketNotifier(xwii_iface_get_fd(iface), QSocketNotifier::Read);
    sn->setEnabled(true);
    connect(sn, SIGNAL(activated(int)), this, SLOT(readReady()));
}


/**
 * @brief BBMonitorWidget::readReady
 *        This function will be called when the balance board interface got readable.
 */
void BBMonitorWidget::readReady()
{
    struct xwii_event event;

    int ret = xwii_iface_dispatch(iface, &event, sizeof(event));
    if (ret)
    {
        if (ret != -EAGAIN)
        {
            qDebug("Error: Read failed with err:%d\n", ret);
            }
    }
    else
    {
        switch (event.type)
        {
            case XWII_EVENT_GONE:
                qDebug("Info: Device gone\n");
                break;
            case XWII_EVENT_BALANCE_BOARD:
                handle_bboard_event(&event);
                break;
        }
    }
}

/**
 * @brief BBMonitorWidget::handle_bboard_event
 *        Handles a new balance board event by updating the queues and center of mass
 *        variables.
 * @param event
 */
void BBMonitorWidget::handle_bboard_event(const struct xwii_event *event)
{
    double w, x, y, z;

    w = event->v.abs[0].x;
    x = event->v.abs[1].x;
    y = event->v.abs[2].x;
    z = event->v.abs[3].x;


    // Update Plots
    queue_w.dequeue();
    queue_w.enqueue(w);

    queue_x.dequeue();
    queue_x.enqueue(x);

    queue_y.dequeue();
    queue_y.enqueue(y);

    queue_z.dequeue();
    queue_z.enqueue(z);

    double sum = w+x+y+z;

    queue_sum.dequeue();
    queue_sum.enqueue(sum);

    if(sum != 0)
    {
        center_of_mass_x[0] = (w+x-y-z)/sum;
        center_of_mass_y[0] = (y+w-z-x)/sum;
    }
    else
    {
        center_of_mass_x[0] = 0;
        center_of_mass_y[0] = 0;
    }

    loadPlot();

    // Write to file
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

    if (csv_file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&csv_file);
        stream << timestamp << "," << w << "," << x << "," << y << "," << z << endl;
        csv_file.close();
    }
}

/**
 * @brief enumerate
 *        List all connected Wii controller in the debug console.
 * @return Number of connected Wii controller.
 */
static int enumerate()
{
    struct xwii_monitor *mon;
    char *ent;
    int num = 0;

    mon = xwii_monitor_new(false, false);
    if (!mon)
    {
        qDebug("Cannot create monitor\n");
        return -EINVAL;
    }

    while ((ent = xwii_monitor_poll(mon)))
    {
        qDebug("  Found device #%d: %s\n", ++num, ent);
        free(ent);
    }

    xwii_monitor_unref(mon);
    return 0;
}
