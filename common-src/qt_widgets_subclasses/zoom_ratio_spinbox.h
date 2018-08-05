#ifndef ZOOM_RATIO_SPINBOX_H
#define ZOOM_RATIO_SPINBOX_H

#include <QDoubleSpinBox>
#include <QWheelEvent>

class ZoomRatioSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    ZoomRatioSpinBox(QWidget *parent = nullptr);

protected:
    // reimplement wheelEvent for more reasonable scale when zoom out
    void wheelEvent(QWheelEvent* event) override;

};

#endif // ZOOM_RATIO_SPINBOX_H
