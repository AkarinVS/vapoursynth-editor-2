#ifndef TIMELINE_AREA_H
#define TIMELINE_AREA_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QObject>
#include <QTime>

#include "timeline.h"
#include "slider.h"


class TimeLineView: public QGraphicsView
{
    Q_OBJECT

public:
    TimeLineView(QWidget * a_pParent = nullptr);

    virtual ~TimeLineView() override;

    enum DisplayMode
    {
        Time,
        Frames,
    };

    void setFrame (int a_frame);
    int frame();

    void setFramesNumber(int a_framesNumber);

    void setFPS(double a_fps);

    void setDisplayMode(TimeLine::DisplayMode a_displayMode);

    void setPlay(bool a_playing);    

    int zoomFactor();
    void setZoomFactor(int a_zoomFactor);

    void centerSliderOnCurrentFrame();

signals:

    void signalFrameChanged(int a_frame);
    void signalJumpToFrame(int a_frame);
    void signalHoverTime(const QTime &a_time);

public slots:

    void slotSetTimeLine(int a_numFrame, int64_t a_fpsNum, int64_t a_fpsDen);

    void slotResizeSceneWidth();

    void slotSetSliderPos();

    void slotSetSliderPosByFrame(int a_frame);

    // bookmarks
    void slotBookmarkCurrentFrame();
    void slotUnbookmarkCurrentFrame();
    void slotGoToPreviousBookmark();
    void slotGoToNextBookmark();

    void slotPreviewAreaKeyPressed(Qt::Key a_enumKey);

private slots:

    void slotFrameChangedHandler(int a_frame);

private:

    QGraphicsScene * m_pScene;
    Slider * m_pSlider;
    TimeLine * m_pTimeLine;

    bool m_playing;
    bool m_mousePressed;

protected:

    void keyPressEvent(QKeyEvent *a_pEvent) override;

    void mousePressEvent(QMouseEvent * a_pEvent) override;

    void mouseReleaseEvent (QMouseEvent * a_pEvent) override;

    void mouseMoveEvent(QMouseEvent * a_pEvent) override;

    void resizeEvent(QResizeEvent *a_pEvent) override;

    void wheelEvent(QWheelEvent *a_pEvent) override;
};

#endif // TIMELINE_AREA_H
