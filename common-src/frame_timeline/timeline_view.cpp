#include "timeline_view.h"
#include <QDebug>

TimeLineView::TimeLineView (QWidget * a_pParent ):
    m_playing(false),
    m_mousePressed(false)
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
            this, &TimeLineView::slotFrameChangedHandler); // forward signal up to frame indicator
    connect(timeLine, &TimeLine::signalTimeLineWidthChanged,
            this, &TimeLineView::slotResizeSceneWidth);
    connect(timeLine, &TimeLine::signalTimeLineWidthChanged,
            this, &TimeLineView::slotSetSliderPos);

    // setup navagation step, can be improve with more predefine values
    timeLine->setStep(1);

    setMouseTracking(true);
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

int TimeLineView::frame()
{
    return timeLine->frame();
}

int TimeLineView::zoomFactor()
{
    return timeLine->zoomFactor();
}

void TimeLineView::setZoomFactor(int a_zoomFactor)
{
    timeLine->setZoomFactor(a_zoomFactor);
}

void TimeLineView::centerSliderOnCurrentFrame()
{
    // center on current frame after resize
    int pos_in_timeline = timeLine->currentFramePos();
    QPointF pos_in_scene = timeLine->mapToScene(pos_in_timeline, 20);
    this->centerOn(pos_in_scene);
}

void TimeLineView::slotSetTimeLine(int a_numFrames, int64_t a_fpsNum, int64_t a_fpsDen)
{
    int current_viewWidth = this->width();
    timeLine->setViewWidth(current_viewWidth);
    timeLine->setBaseWidth((current_viewWidth));

    setFramesNumber(a_numFrames);

    if(a_fpsDen == 0)
        setFPS(0.0); // for zooming timeline
    else
        setFPS(double(a_fpsNum) / double(a_fpsDen));
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

void TimeLineView::slotFrameChangedHandler(int a_frame)
{
//    qDebug() << "signal send from timeline view, frame: " << a_frame;
    slotSetSliderPosByFrame(a_frame); // set slider position
    emit signalFrameChanged(a_frame); // forward signal up
}

void TimeLineView::mousePressEvent(QMouseEvent * a_pEvent)
{
    m_mousePressed = true;

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

void TimeLineView::mouseReleaseEvent(QMouseEvent * a_pEvent)
{
    m_mousePressed = false;
    QGraphicsView::mouseReleaseEvent(a_pEvent);
}

void TimeLineView::mouseMoveEvent(QMouseEvent *a_pEvent)
{
    QPoint mouse_pos = a_pEvent->pos();

    // map qpoint position in timeLineView to position relative to timeline item, important for zooming
    QPointF pos_in_scene = this->mapToScene(mouse_pos);
    QPointF pos_in_item = timeLine->mapFromScene(pos_in_scene);

    int dest_pos = int(pos_in_item.x());

    int frameOnPos = timeLine->posToFrame(dest_pos);

    double fps = timeLine->fps();
    int milliSeconds = int((double(frameOnPos) / fps) * 1000);
    QTime time = QTime::fromMSecsSinceStartOfDay(milliSeconds);
    emit signalHoverTime(time);

    if (m_mousePressed) {
        // set frame in timeline to display
        timeLine->setFrameByPos(dest_pos);

        // retrieve the frame position in timeline to set slider position
        int timeLineFramePos = timeLine->currentFramePos();

        slider->setPos(timeLineFramePos, slider->pos().y());        
    }


    QGraphicsView::mouseMoveEvent(a_pEvent);
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

    centerSliderOnCurrentFrame();
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
            centerSliderOnCurrentFrame();
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

