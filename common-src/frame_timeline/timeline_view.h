#ifndef TIMELINE_AREA_H
#define TIMELINE_AREA_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QObject>

#include "timeline.h"
#include "slider.h"


class TimeLineView: public QGraphicsView
{
    Q_OBJECT

public:
    TimeLineView(QWidget * a_pParent = nullptr);

    virtual ~TimeLineView() override;

    void setFrame (int a_frame);

    void setFramesNumber(int a_framesNumber);

    void setFPS(double a_fps);

    void setDisplayMode(TimeLine::DisplayMode a_displayMode);

    void setPlay(bool a_playing);

signals:

    void signalFrameChanged(int a_frame);
    void signalJumpToFrame(int a_frame);

public slots:

    void slotFrameChanged(int a_frame);

    void slotResizeSceneWidth();

    void slotSetSliderPos();

    void slotSetSliderPosByFrame(int a_frame);

    // bookmarks
    void slotBookmarkCurrentFrame();
    void slotUnbookmarkCurrentFrame();
    void slotGoToPreviousBookmark();
    void slotGoToNextBookmark();

private:

    QGraphicsScene *scene;
    Slider *slider;
    TimeLine *timeLine;

    int m_maxFrame;

    bool m_playing;

    bool m_mousePressed;

protected:

    void mousePressEvent(QMouseEvent * a_event) override;

    void mouseReleaseEvent (QMouseEvent * a_pEvent) override;

    void mouseMoveEvent(QMouseEvent * a_pEvent) override;

    void resizeEvent(QResizeEvent *a_pEvent) override;

    void wheelEvent(QWheelEvent *a_pEvent) override;
};

#endif // TIMELINE_AREA_H
