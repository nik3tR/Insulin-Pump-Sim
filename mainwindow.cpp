#include "mainwindow.h"
#include "src/widgets/pumpsimulatormainwidget.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("t:slim X2 Insulin Pump Simulator");
    auto* sim = new PumpSimulatorMainWidget(this);
    setCentralWidget(sim);
    resize(650, 550);
}
