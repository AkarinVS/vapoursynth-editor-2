#include "zoom_ratio_spinbox.h"
#include <math.h>

#include <QDebug>
#include <QStyleOptionSpinBox>

ZoomRatioSpinBox::ZoomRatioSpinBox(QWidget *parent): QDoubleSpinBox(parent)
{
    // use custom lineedit for single click to highlight all text
    a_pLineEdit = new SpinboxExtendedLineEdit(this);
    setLineEdit(a_pLineEdit);

    // other settings were set in the ui
}

// END OF ZoomRatioSpinBox::ZoomRatioSpinBox(QWidget *parent): QDoubleSpinBox(parent)
//==============================================================================

// set zoom ratio to +/-0.2x when ratio is under 1x
void ZoomRatioSpinBox::wheelEvent(QWheelEvent * a_pEvent)
{
    QDoubleSpinBox::wheelEvent(a_pEvent);

    QPoint delta = a_pEvent->angleDelta();

    if(value() > 1.0 && delta.y() > 0)
        stepUp();

    if(value() > 1.0 && delta.y() < 0)
        stepDown();

    a_pEvent->ignore();
}

// END OF ZoomRatioSpinBox::wheelEvent(QWheelEvent * event)
//==============================================================================

void ZoomRatioSpinBox::mousePressEvent(QMouseEvent * a_pEvent)
{
    QDoubleSpinBox::mousePressEvent(a_pEvent);

    QStyleOptionSpinBox opt;
    this->initStyleOption(&opt);

    // ARROW UP IS PRESSED
    if(this->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp).contains(a_pEvent->pos()) )
        stepUp();

    // ARROW DOWN IS PRESSED
    if( this->style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxDown).contains(a_pEvent->pos()) )
        stepDown();

    a_pEvent->ignore();
}

// END OF ZoomRatioSpinBox::mousePressEvent(QMouseEvent * event)
//==============================================================================

void ZoomRatioSpinBox::keyPressEvent(QKeyEvent * a_pEvent)
{
    QDoubleSpinBox::keyPressEvent(a_pEvent);

    if (a_pEvent->key() == Qt::Key_Up) {
        stepUp();
    }

    if (a_pEvent->key() == Qt::Key_Down)
        stepDown();

    a_pEvent->ignore();
}

// END OF void ZoomRatioSpinBox::keyPressEvent(QKeyEvent *a_pEvent)
//==============================================================================

void ZoomRatioSpinBox::stepUp()
{
    // +-1 when value > 1, +-0.2 when value < 1
    // the +0.2, -0.2 is here to make zooming from preview area work
    if (value() >= 1.0)
        setValue(ceil(value()+0.2));
    else
        setValue(value()+0.2);
}

// END OF void ZoomRatioSpinBox::stepUp)
//==============================================================================

void ZoomRatioSpinBox::stepDown()
{
    if (value() > 1.0)
        setValue(floor(value()-0.2));
    else {
        setValue(value()-0.2);
    }
}

// END OF void ZoomRatioSpinBox::stepDown)
//==============================================================================
