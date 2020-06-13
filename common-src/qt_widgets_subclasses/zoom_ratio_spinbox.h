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
    virtual void wheelEvent(QWheelEvent * a_pEvent) override;
    virtual void mousePressEvent(QMouseEvent * a_pEvent) override;
    virtual void keyPressEvent(QKeyEvent * a_pEvent) override;

private:
    double m_zoomRatio;

public slots:
    void stepUp();
    void stepDown();
};

#endif // ZOOM_RATIO_SPINBOX_H
