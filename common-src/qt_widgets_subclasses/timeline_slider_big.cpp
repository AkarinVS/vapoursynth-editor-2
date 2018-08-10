#include "timeline_slider_big.h"

#include <QWheelEvent>
#include <QStyle>
#include <QStyleOptionSlider>

TimeLineSliderBig::TimeLineSliderBig(QWidget * parent): QSlider(parent)
{

}

// END OF TimeLineSliderBig::TimeLineSliderBig(QWidget * parent): QSlider(parent)
//==============================================================================

void TimeLineSliderBig::wheelEvent(QWheelEvent * event)
{        
    int deltaY = event->angleDelta().y();
    int lastValue = value();

    if (deltaY > 0)
        setValue(lastValue + 1);
    else
        setValue(lastValue - 1);

    QSlider::wheelEvent(event);
}

// END OF void TimeLineSliderBig::wheelEvent(QWheelEvent * event)
//==============================================================================

void TimeLineSliderBig::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_sliderPressed = true;
        emit sliderPressed();

        int pos = event->x();
        int newFrame = posToFrame(pos);
        setValue(newFrame);
    }

    QSlider::mousePressEvent(event);
}

// END OF void TimeLineSliderBig::mousePressEvent(QMouseEvent * event)
//==============================================================================

void TimeLineSliderBig::mouseReleaseEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(m_sliderMoved){
            emit valueChanged(value());
        }

        m_sliderPressed = false;
        m_sliderMoved = false;
        emit sliderReleased();
    }
    QSlider::mouseReleaseEvent(event);
}

// END OF void TimeLineSliderBig::mouseReleaseEvent(QMouseEvent * event)
//==============================================================================

int TimeLineSliderBig::posToFrame(int pos) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    double halfHandleWidth = (0.5 * handle.width()) + 0.5; // Correct rounding
    int adaptedPosX = pos;
    if ( adaptedPosX < halfHandleWidth )
            adaptedPosX = halfHandleWidth;
    if ( adaptedPosX > width() - halfHandleWidth )
            adaptedPosX = width() - halfHandleWidth;

    // get new dimensions accounting for slider handle width
    double newWidth = (width() - halfHandleWidth) - halfHandleWidth;
    double normalizedPosition = (adaptedPosX - halfHandleWidth)  / newWidth ;

    int newVal = minimum() + ((maximum()-minimum()) * normalizedPosition);
    return newVal;
}

// END OF int TimeLineSliderBig::posToFrame(int pos) const
//==============================================================================
