#include "slider.h"

#include <QBrush>
#include <QPainter>

Slider::Slider(QGraphicsItem * parent)
{

}

Slider::~Slider()
{

}

QRectF Slider::boundingRect() const
{
    return QRectF(0, 0, 1, 120);
}

void Slider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QBrush blackBrush(Qt::black);
    painter->setBrush(blackBrush);
    painter->drawLine(0,0,1,120);
}
