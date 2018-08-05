#include "zoom_ratio_spinbox.h"

ZoomRatioSpinBox::ZoomRatioSpinBox(QWidget *parent): QDoubleSpinBox(parent)
{

}

// END OF ZoomRatioSpinBox::ZoomRatioSpinBox(QWidget *parent): QDoubleSpinBox(parent)
//==============================================================================
void ZoomRatioSpinBox::wheelEvent(QWheelEvent * event)
{
    QDoubleSpinBox::wheelEvent(event);

    this->setSingleStep(0.2);
    double zoomRatio = value();
    int delta = event->delta();

    if(zoomRatio > 1.0 && delta > 0)
    {
        zoomRatio = ceil(zoomRatio);
        setValue(zoomRatio);
    }

    if(zoomRatio > 1.0 && delta < 0)
    {
        zoomRatio = floor(zoomRatio);
        setValue(zoomRatio);
    }

    event->ignore(); // important, set this to not override original's behavior
}
// END OF ZoomRatioSpinBox::wheelEvent(QWheelEvent * event)
//==============================================================================
