#include "mainwindow.h"
#include "pumpsimulatormainwidget.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("t:slim X2 Insulin Pump Simulator");
    PumpSimulatorMainWidget* mainWidget = new PumpSimulatorMainWidget(this);
    setCentralWidget(mainWidget);
    resize(650, 550);
}
