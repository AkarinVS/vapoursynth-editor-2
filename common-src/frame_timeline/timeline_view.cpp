#include "timeline_view.h"
#include <QDebug>

TimeLineView::TimeLineView (QWidget * a_pParent )
{
    // setup scene and timeline items
    scene = new QGraphicsScene(this);
    timeLine = new TimeLine();
    slider = new Slider();

    timeLine->setCacheMode(QGraphicsItem::ItemCoordinateCache);

    scene->addItem(timeLine);
    scene->addItem(slider);

    this->setScene(scene);
    scene->setBackgroundBrush(QColor("#e7f1f3"));

    connect(timeLine, &TimeLine::signalFrameChanged,
            this, &TimeLineView::signalFrameChanged); // forward signal to preview dialog
    connect(timeLine, &TimeLine::signalFrameChanged,
            this, &TimeLineView::slotSetSliderPosByFrame);
//    connect(timeLine, &TimeLine::signalFrameChanged,
//            this, &TimeLineView::slotSetSliderPosByFrame);
    connect(timeLine, &TimeLine::signalTimeLineWidthChanged,
            this, &TimeLineView::slotResizeSceneWidth);
    connect(timeLine, &TimeLine::signalTimeLineWidthChanged,
            this, &TimeLineView::slotSetSliderPos);

    // setup navagation step, can be improve with more predefine values
    timeLine->setStep(1);

}

TimeLineView::~TimeLineView()
{

}

void TimeLineView::setFrame(int a_frame)
{    
    timeLine->setFrame(a_frame);
}

void TimeLineView::setFramesNumber(int a_framesNumber)
{
    // passthrough function, set toal number of frames to timeline
    timeLine->setFramesNumber(a_framesNumber);
}

void TimeLineView::setFPS(double a_fps)
{
    timeLine->setFPS(a_fps);
}

void TimeLineView::setDisplayMode(TimeLine::DisplayMode a_displayMode)
{
    timeLine->setDisplayMode(a_displayMode);
}

void TimeLineView::setPlay(bool a_playing)
{
    m_playing = a_playing;
}

void TimeLineView::slotFrameChanged(int a_frame)
{
    emit signalFrameChanged(a_frame);
}

void TimeLineView::slotResizeSceneWidth()
{
    QRectF rect = timeLine->boundingRect();
    scene->setSceneRect(rect);
}

void TimeLineView::slotSetSliderPos()
{
    int pos = timeLine->currentFramePos();
    slider->setPos(pos, slider->pos().y());
}

void TimeLineView::slotSetSliderPosByFrame(int a_frame)
{        
    // get position from timeline and set it to slider
    int pos = timeLine->frameToPos(a_frame);
    slider->setPos(pos, slider->pos().y());
}

void TimeLineView::slotBookmarkCurrentFrame()
{
    timeLine->addBookmark(timeLine->frame());
}

void TimeLineView::slotUnbookmarkCurrentFrame()
{
    timeLine->removeBookmark(timeLine->frame());
}

void TimeLineView::slotGoToPreviousBookmark()
{
    timeLine->slotGoToPreviousBookmark();
}

void TimeLineView::slotGoToNextBookmark()
{
    timeLine->slotGoToNextBookmark();
}

void TimeLineView::mouseReleaseEvent(QMouseEvent * a_pEvent)
{
    QGraphicsView::mouseReleaseEvent(a_pEvent);
}

void TimeLineView::resizeEvent(QResizeEvent *a_pEvent)
{
    QGraphicsView::resizeEvent(a_pEvent);

    // get current width of view
    int current_viewWidth = this->width();

//    qDebug() << "current view width: " << current_viewWidth;

    timeLine->setViewWidth(current_viewWidth);
    timeLine->setBaseWidth((current_viewWidth));
    timeLine->reloadZoomFactor();

    // center on current frame after resize
    int pos_in_timeline = timeLine->currentFramePos();
    QPointF pos_in_scene = timeLine->mapToScene(pos_in_timeline, 20);
    this->centerOn(pos_in_scene);
}

void TimeLineView::wheelEvent(QWheelEvent *a_pEvent)
{

    QPoint delta = a_pEvent->angleDelta();

    if(a_pEvent->modifiers() != Qt::NoModifier)
    {
        // zooming timeline
        if (a_pEvent->modifiers() == Qt::ControlModifier) {
            if (delta.y() > 0) {
                timeLine->increaseZoomFactor();
            } else {
                timeLine->decreaseZoomFactor();
            }

            // retrieve mouse position on of current frame after resize, then map it to pos in scene
            // and then center on pos
            int pos_in_timeline = timeLine->currentFramePos();
            QPointF pos_in_scene = timeLine->mapToScene(pos_in_timeline, 20);
            this->centerOn(pos_in_scene);
        }
        return;
    }

    // disable scroll when video is playing
    if (!m_playing) {
        if(delta.x() == 0)
        {
            if(delta.y() < 0)
                timeLine->slotStepDown();
            else
                timeLine->slotStepUp();
        }
        else
        {
            // for mouse with horizontal scroll
            if(delta.x() < 0)
                timeLine->slotStepDown();
            else
                timeLine->slotStepUp();
        }
    }

    QGraphicsView::wheelEvent(a_pEvent);
    a_pEvent->accept();

}

void TimeLineView::mousePressEvent(QMouseEvent * a_pEvent)
{
    QGraphicsView::mousePressEvent(a_pEvent);
    QPoint mouse_pos = a_pEvent->pos();

    // map qpoint position in timeLineView to position relative to timeline item, important for zooming
    QPointF pos_in_scene = this->mapToScene(mouse_pos);
    QPointF pos_in_item = timeLine->mapFromScene(pos_in_scene);

    int dest_pos = int(pos_in_item.x());


    // get frame pos and move slider to frame position
    // snap slider to start
    if (pos_in_item.x() < 10) {
        dest_pos = 0;
    }

    // snap slider to end
    if (pos_in_item.x() > timeLine->viewWidth() - 10) {
        dest_pos = timeLine->viewWidth();
    }

    // set frame in timeline to display
    timeLine->setFrameByPos(dest_pos);

    // retrieve the frame position in timeline to set slider position
    int timeLineFramePos = timeLine->currentFramePos();
    slider->setPos(timeLineFramePos, slider->pos().y());

    int destFrame = timeLine->frame();
    if (m_playing) {
        emit signalJumpToFrame(destFrame);
    }
}
