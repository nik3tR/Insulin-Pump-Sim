#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QLineSeries>

QT_CHARTS_USE_NAMESPACE

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = nullptr);

    // Call this to update the graph with the current glucose level.
    void updateGraph(double currentGlucoseLevel);

    // Clear the graph (if needed).
    void clearGraph();

private:
    QChart* m_chart;
    QScatterSeries* m_graph_points;
    QScatterSeries* m_predicted_points;
    QSplineSeries* m_graph_line;
    QChartView* m_chartView;
};

#endif // GRAPHWIDGET_H
