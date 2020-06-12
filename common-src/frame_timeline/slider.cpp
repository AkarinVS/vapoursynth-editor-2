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
    return QRectF(0, 0, 1, 100);
}

void Slider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect = boundingRect();

    QBrush blackBrush(Qt::black);
    painter->setBrush(blackBrush);
    painter->drawRect(rect);
}
