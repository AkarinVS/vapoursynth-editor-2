#include "frame_painter.h"

#include <QPainter>

FramePainter::FramePainter(QWidget *parent) : QWidget(parent),
  m_ratio(1.0)
{    
}

void FramePainter::paintEvent(QPaintEvent *a_pEvent)
{
    QPainter painter(this);
    painter.scale(m_ratio, m_ratio);
    painter.drawPixmap(0, 0, m_framePixmap.width(),m_framePixmap.height(), m_framePixmap);
    painter.end();
}

void FramePainter::drawFrame(const QPixmap &a_framePixmap)
{
    m_framePixmap = a_framePixmap;
    update();
}

QPixmap FramePainter::pixmap()
{
    return m_framePixmap;
}

void FramePainter::setRatio(const qreal a_ratio)
{
    if (m_ratio != a_ratio)
        m_ratio = a_ratio;
    update();
}
