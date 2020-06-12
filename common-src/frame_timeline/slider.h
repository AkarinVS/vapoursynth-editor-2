#ifndef SLIDER_H
#define SLIDER_H

#include <QGraphicsRectItem>

class Slider: public QGraphicsRectItem
{
public:
    Slider(QGraphicsItem * parent = nullptr);

    virtual ~Slider() override;

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

};

#endif // SLIDER_H
