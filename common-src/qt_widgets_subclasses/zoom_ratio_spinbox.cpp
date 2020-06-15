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

    m_zoomRatio = value();
    QPoint delta = a_pEvent->angleDelta();

    if(m_zoomRatio > 1.0 && delta.y() > 0)
        stepUp();

    if(m_zoomRatio > 1.0 && delta.y() < 0)
        stepDown();

    a_pEvent->ignore();
}

// END OF ZoomRatioSpinBox::wheelEvent(QWheelEvent * event)
//==============================================================================

void ZoomRatioSpinBox::mousePressEvent(QMouseEvent * a_pEvent)
{
    QDoubleSpinBox::mousePressEvent(a_pEvent);

    m_zoomRatio = value();

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

    m_zoomRatio = value();

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
    if (m_zoomRatio > 1.0)
        setValue(ceil(m_zoomRatio));
}

// END OF void ZoomRatioSpinBox::stepUp)
//==============================================================================

void ZoomRatioSpinBox::stepDown()
{
    if (m_zoomRatio > 1.0)
        setValue(floor(m_zoomRatio));
}

// END OF void ZoomRatioSpinBox::stepDown)
//==============================================================================
