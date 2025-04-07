#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class PumpSimulatorMainWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
};
#endif // MAINWINDOW_H
