#include "line_painter.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

LinePainter::LinePainter(QWidget *parent) : QWidget(parent),
    m_scaleFactor(1.0),
    m_minX(0.0),
    m_minY(0.0),
    m_maxX(0.0),
    m_maxY(0.0)
{
    setMouseTracking(true);
}

void LinePainter::setSelectionPixmap(const QPixmap & a_pixmap)
{
    if (m_selectionPixmap.width() > 0 && m_selectionPixmap.width() != a_pixmap.width()) {
        m_scaleFactor = double(a_pixmap.width()) / double(m_selectionPixmap.width());
    }

    m_initialPixmapCopy = a_pixmap;
    m_selectionPixmap = a_pixmap;

    // update selection rect if it existed
    updateSelectionRect();
    update();
}

void LinePainter::updateSelectionRect()
{
    QPainter painter(&m_selectionPixmap);
    painter.setPen(QPen(Qt::red, 2));

    m_minX *= m_scaleFactor;
    m_maxX *= m_scaleFactor;
    m_minY *= m_scaleFactor;
    m_maxY *= m_scaleFactor;

    QRect selectionRect(int(m_minX), int(m_minY), int(m_maxX - m_minX), int(m_maxY - m_minY));
    painter.drawRect(selectionRect);
}

void LinePainter::setSelectionTool(SelectionTools a_selectionTool)
{
    m_selectionTool = a_selectionTool;
}

QVector<double> LinePainter::selectionRectXY()
{
//    qDebug() << m_minX;
    return QVector({m_minX, m_minY, m_maxX, m_maxY});
}

void LinePainter::mousePressEvent(QMouseEvent *a_pEvent)
{
    m_drawing = false;
    if (a_pEvent->button() == Qt::LeftButton) {
        switch (m_selectionTool) {
        case Rectangle:
            break;
        case LazyRectangle:
            m_drawing = true;
            m_minX = a_pEvent->pos().x();
            m_maxX = a_pEvent->pos().x();
            m_minY = a_pEvent->pos().y();
            m_maxY = a_pEvent->pos().y();

            m_pathPoints.append(a_pEvent->pos());
            m_lastPoint = a_pEvent->pos();
            break;
        }
    }
}

void LinePainter::mouseMoveEvent(QMouseEvent *a_pEvent)
{
    if ((a_pEvent->buttons() & Qt::LeftButton) && m_drawing) {
        m_pathPoints.append(a_pEvent->pos());
        drawPointTo(a_pEvent->pos());
    } else {
        QWidget::mouseMoveEvent(a_pEvent);
    }
}

void LinePainter::mouseReleaseEvent(QMouseEvent *a_pEvent)
{
    if (a_pEvent->button() == Qt::LeftButton && m_drawing) {
        m_pathPoints.append(a_pEvent->pos());
        drawPointTo(a_pEvent->pos());

        createSelectedRect();
        m_drawing = false;
    }
}

void LinePainter::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0,0,m_selectionPixmap);
    painter.end();
}

void LinePainter::drawPointTo(const QPointF &a_endPoint)
{
    QPainter painter(&m_selectionPixmap);
    painter.setPen(QPen(Qt::red, 2));

    painter.drawLine(m_lastPoint, a_endPoint);

    update();
    m_lastPoint = a_endPoint;
}

void LinePainter::createSelectedRect()
{
    if (m_pathPoints.isEmpty()) return;

    /* filter out vector to get min/max X and Y */
    for (auto &point : m_pathPoints) {
        if (point.x() < m_minX ) {
            m_minX = point.x();
        }
        if (point.x() > m_maxX) {
            m_maxX = point.x();
        }

        if (point.y() < m_minY) {
            m_minY = point.y();
        }

        if (point.y() > m_maxY) {
            m_maxY = point.y();
        }
    }
    if (m_minX == m_maxX || m_minY == m_maxY) {
        m_pathPoints.clear();
    }

    /* create a rectangle from the four points and clear vector */
    m_selectionPixmap = m_initialPixmapCopy; // use a fresh pixmap from the initial copy
    QPainter painter(&m_selectionPixmap);
    painter.setPen(QPen(Qt::red, 2));

    painter.drawRect(QRect(int(m_minX), int(m_minY), int(m_maxX - m_minX), int(m_maxY - m_minY)));
    m_pathPoints.clear();
}
