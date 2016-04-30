#include "bbmonitorwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BBMonitorWidget w;
    w.show();

    return a.exec();
}
