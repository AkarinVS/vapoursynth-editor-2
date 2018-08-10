#ifndef TIMELINE_SLIDER_BIG_H
#define TIMELINE_SLIDER_BIG_H

#include <QSlider>

class TimeLineSliderBig : public QSlider
{
public:
    TimeLineSliderBig(QWidget * a_pParent = nullptr);

protected:

    void wheelEvent(QWheelEvent * event);

    void mousePressEvent(QMouseEvent * event);

    void mouseReleaseEvent(QMouseEvent * event);

private:

    int posToFrame(int a_pos) const;

    bool m_sliderPressed;

    bool m_sliderMoved;
};

#endif // TIMELINE_SLIDER_BIG_H
