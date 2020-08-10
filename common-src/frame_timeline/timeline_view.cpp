#include "timeline_view.h"
#include <QDebug>

TimeLineView::TimeLineView (QWidget * a_pParent ):
    m_playing(false),
    m_mousePressed(false),
    m_pSlider(nullptr),
    m_pScene(nullptr),
    m_pTimeLine(nullptr)
{
    // setup scene and timeline items
    m_pScene = new QGraphicsScene(this);
    m_pTimeLine = new TimeLine();
    m_pSlider = new Slider();

    m_pTimeLine->setCacheMode(QGraphicsItem::ItemCoordinateCache);

    m_pScene->addItem(m_pTimeLine);
    m_pScene->addItem(m_pSlider);

    this->setScene(m_pScene);
    m_pScene->setBackgroundBrush(QColor("#e7f1f3"));

    connect(m_pTimeLine, &TimeLine::signalFrameChanged,
            this, &TimeLineView::slotFrameChangedHandler); // forward signal up to frame indicator
    connect(m_pTimeLine, &TimeLine::signalTimeLineWidthChanged,
            this, &TimeLineView::slotResizeSceneWidth);
    connect(m_pTimeLine, &TimeLine::signalTimeLineWidthChanged,
            this, &TimeLineView::slotSetSliderPos);

    // setup navagation step, can be improve with more predefine values
    m_pTimeLine->setStep(1);

    setMouseTracking(true);
}

TimeLineView::~TimeLineView()
{

}

void TimeLineView::setFrame(int a_frame)
{    
    m_pTimeLine->setFrame(a_frame);
}

void TimeLineView::setFramesNumber(int a_framesNumber)
{
    m_pTimeLine->setFramesNumber(a_framesNumber);
}

void TimeLineView::setFPS(double a_fps)
{
    m_pTimeLine->setFPS(a_fps);
}

void TimeLineView::setDisplayMode(TimeLine::DisplayMode a_displayMode)
{
    m_pTimeLine->setDisplayMode(a_displayMode);
}

void TimeLineView::setPlay(bool a_playing)
{
    m_playing = a_playing;
}

int TimeLineView::frame()
{
    return m_pTimeLine->frame();
}

int TimeLineView::zoomFactor()
{
    return m_pTimeLine->zoomFactor();
}

void TimeLineView::setZoomFactor(int a_zoomFactor)
{
    m_pTimeLine->setZoomFactor(a_zoomFactor);
}

void TimeLineView::centerSliderOnCurrentFrame()
{
    // center on current frame after resize
    int pos_in_timeline = m_pTimeLine->currentFramePos();
    QPointF pos_in_scene = m_pTimeLine->mapToScene(pos_in_timeline, 20);
    this->centerOn(pos_in_scene);
}

void TimeLineView::slotSetTimeLine(int a_numFrames, int64_t a_fpsNum, int64_t a_fpsDen)
{
    int current_viewWidth = this->width();
    m_pTimeLine->setViewWidth(current_viewWidth);
    m_pTimeLine->setBaseWidth((current_viewWidth));

    setFramesNumber(a_numFrames);

    if(a_fpsDen == 0)
        setFPS(0.0); // for zooming timeline
    else
        setFPS(double(a_fpsNum) / double(a_fpsDen));
}

void TimeLineView::slotResizeSceneWidth()
{
    QRectF rect = m_pTimeLine->boundingRect();
    m_pScene->setSceneRect(rect);
}

void TimeLineView::slotSetSliderPos()
{
    int pos = m_pTimeLine->currentFramePos();
    m_pSlider->setPos(pos, m_pSlider->pos().y());
}

void TimeLineView::slotSetSliderPosByFrame(int a_frame)
{        
    // get position from timeline and set it to slider
    int pos = m_pTimeLine->frameToPos(a_frame);
    m_pSlider->setPos(pos, m_pSlider->pos().y());
}

void TimeLineView::slotBookmarkCurrentFrame()
{
    m_pTimeLine->addBookmark(m_pTimeLine->frame());
}

void TimeLineView::slotUnbookmarkCurrentFrame()
{
    m_pTimeLine->removeBookmark(m_pTimeLine->frame());
}

void TimeLineView::slotGoToPreviousBookmark()
{
    m_pTimeLine->slotGoToPreviousBookmark();
}

void TimeLineView::slotGoToNextBookmark()
{
    m_pTimeLine->slotGoToNextBookmark();
}

void TimeLineView::slotPreviewAreaKeyPressed(Qt::Key a_enumKey)
{
    if (a_enumKey == Qt::Key_Left)
        m_pTimeLine->slotStepDown();
    else if (a_enumKey == Qt::Key_Right)
        m_pTimeLine->slotStepUp();
    else if (a_enumKey == Qt::Key_Home)
        setFrame(0);
    else if (a_enumKey == Qt::Key_End)
        setFrame(m_pTimeLine->maxFrame());
}

void TimeLineView::slotFrameChangedHandler(int a_frame)
{
    slotSetSliderPosByFrame(a_frame); // set slider position
    emit signalFrameChanged(a_frame); // forward signal up
}

void TimeLineView::keyPressEvent(QKeyEvent *a_pEvent)
{
    int key = a_pEvent->key();

    if (key == Qt::Key_Left) {
        m_pTimeLine->slotStepDown();
    }
    else if (key == Qt::Key_Right) {
        m_pTimeLine->slotStepUp();
    }
    else if (key == Qt::Key_Home)
        setFrame(0);
    else if (key == Qt::Key_End)
        setFrame(m_pTimeLine->maxFrame());
    else
        QGraphicsView::keyPressEvent(a_pEvent);
}

void TimeLineView::mousePressEvent(QMouseEvent * a_pEvent)
{
    m_mousePressed = true;

    QGraphicsView::mousePressEvent(a_pEvent);
    QPoint mouse_pos = a_pEvent->pos();

    // map qpoint position in timeLineView to position relative to timeline item, important for zooming
    QPointF pos_in_scene = this->mapToScene(mouse_pos);
    QPointF pos_in_item = m_pTimeLine->mapFromScene(pos_in_scene);

    int dest_pos = int(pos_in_item.x());

    // get frame pos and move slider to frame position
    // snap slider to start
    if (pos_in_item.x() < 10) {
        dest_pos = 0;
    }

    // snap slider to end
    if (pos_in_item.x() > m_pTimeLine->viewWidth() - 10) {
        dest_pos = m_pTimeLine->viewWidth();
    }

    // set frame in timeline to display
    m_pTimeLine->setFrameByPos(dest_pos);

    // retrieve the frame position in timeline to set slider position
    int timeLineFramePos = m_pTimeLine->currentFramePos();
    m_pSlider->setPos(timeLineFramePos, m_pSlider->pos().y());

    int destFrame = m_pTimeLine->frame();
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
    QPointF pos_in_item = m_pTimeLine->mapFromScene(pos_in_scene);

    int dest_pos = int(pos_in_item.x());

    int frameOnPos = m_pTimeLine->posToFrame(dest_pos);

    double fps = m_pTimeLine->fps();
    int milliSeconds = int((double(frameOnPos) / fps) * 1000);
    QTime time = QTime::fromMSecsSinceStartOfDay(milliSeconds);
    emit signalHoverTime(time);

    if (m_mousePressed) {
        // set frame in timeline to display
        m_pTimeLine->setFrameByPos(dest_pos);

        // retrieve the frame position in timeline to set slider position
        int timeLineFramePos = m_pTimeLine->currentFramePos();

        m_pSlider->setPos(timeLineFramePos, m_pSlider->pos().y());
    }

    QGraphicsView::mouseMoveEvent(a_pEvent);
}

void TimeLineView::resizeEvent(QResizeEvent *a_pEvent)
{
    QGraphicsView::resizeEvent(a_pEvent);

    int current_viewWidth = this->width();
    m_pTimeLine->setViewWidth(current_viewWidth);
    m_pTimeLine->setBaseWidth((current_viewWidth));
    m_pTimeLine->reloadZoomFactor();

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
                m_pTimeLine->increaseZoomFactor();
            } else {
                m_pTimeLine->decreaseZoomFactor();
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
                m_pTimeLine->slotStepDown();
            else
                m_pTimeLine->slotStepUp();
        }
        else
        {
            // for mouse with horizontal scroll
            if(delta.x() < 0)
                m_pTimeLine->slotStepDown();
            else
                m_pTimeLine->slotStepUp();
        }
    }

    QGraphicsView::wheelEvent(a_pEvent);
    a_pEvent->accept();
}

