#ifndef ZOOM_RATIO_SPINBOX_H
#define ZOOM_RATIO_SPINBOX_H

#include <QDoubleSpinBox>
#include <QWheelEvent>

#include "spinbox_extended_lineedit.h"

class ZoomRatioSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    ZoomRatioSpinBox(QWidget *parent = nullptr);

protected:
    virtual void wheelEvent(QWheelEvent * a_pEvent) override;
    virtual void mousePressEvent(QMouseEvent * a_pEvent) override;
    virtual void keyPressEvent(QKeyEvent * a_pEvent) override;

private:

    SpinboxExtendedLineEdit * a_pLineEdit;

public slots:
    void stepUp();
    void stepDown();
};

#endif // ZOOM_RATIO_SPINBOX_H
