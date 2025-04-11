#include "graphwidget.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QLineSeries>

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
{
    // Create the series for data points and predictions.
    m_graph_points = new QScatterSeries();
    m_graph_points->setMarkerSize(11);
    m_graph_points->setColor(Qt::black);

    m_predicted_points = new QScatterSeries();
    m_predicted_points->setMarkerSize(11);
    m_predicted_points->setColor(Qt::gray);

    m_graph_line = new QSplineSeries();
    m_graph_line->setColor(Qt::blue);

    // Create a vertical line.
    QLineSeries* verticalLine = new QLineSeries();
    verticalLine->append(0, 0);
    verticalLine->append(0, 15);
    verticalLine->setColor(Qt::green);

    // Create and configure the chart.
    m_chart = new QChart();
    m_chart->addSeries(m_graph_points);
    m_chart->addSeries(m_predicted_points);
    m_chart->addSeries(m_graph_line);
    m_chart->addSeries(verticalLine);
    m_chart->createDefaultAxes();
    m_chart->legend()->setVisible(false);

    // Set fixed axes ranges (adjust if desired)
    m_chart->axes()[0]->setRange(-6, 2);
    m_chart->axes()[1]->setRange(2, 11);
    m_chart->axes()[0]->setTitleText("Time (h)");
    m_chart->axes()[1]->setTitleText("Glucose Level (mmol/L)");

    // Create the chart view.
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumSize(QSize(500,350));

    // Setup a layout that contains the chart view.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_chartView);
    setLayout(layout);
}

void GraphWidget::updateGraph(double currentGlucoseLevel)
{
    // Shift existing points to the left and remove those out of range.
    for (int i = 0; i < m_graph_points->count(); ++i) {
        QPointF pt = m_graph_points->at(i);
        if (pt.x() < -6) {
            m_graph_points->remove(i);
            --i;
            continue;
        }
        m_graph_points->replace(i, pt.x() - 0.5, pt.y());
    }

    // Append new glucose level at x = 0.
    m_graph_points->append(QPointF(0, currentGlucoseLevel));
    m_predicted_points->clear();

    // If there are at least 3 points, calculate and add predicted points.
    if (m_graph_points->count() >= 3) {
        QPointF p0 = m_graph_points->at(m_graph_points->count() - 1);
        QPointF p1 = m_graph_points->at(m_graph_points->count() - 2);
        QPointF p2 = m_graph_points->at(m_graph_points->count() - 3);
        QPointF avgDiff = ((p1 - p0) + (p2 - p1)) * 0.5;

        m_predicted_points->append(p0 - avgDiff);
        m_predicted_points->append(p0 - 2 * avgDiff);
        m_predicted_points->append(p0 - 3 * avgDiff);
    }

    // Update graph line with the latest points.
    m_graph_line->clear();
    m_graph_line->append(m_graph_points->points());
    m_graph_line->append(m_predicted_points->points());
}

void GraphWidget::clearGraph()
{
    m_graph_points->clear();
    m_predicted_points->clear();
    m_graph_line->clear();
}
