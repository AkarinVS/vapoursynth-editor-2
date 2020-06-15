#include "timeline.h"
#include "../helpers.h"

#include <math.h>
#include <QDebug>

TimeLine::TimeLine(QWidget * a_pParent)
{
    m_viewWidth = 1000;
    m_baseWidth = 1000;
    m_zoomFactor = 1;
    m_accuScaleMultiplier = 1.0;
    m_widthChanged = true;
    m_widthPerSegment = 192;

    setPos(mapToParent(0,0));

    connect(this, &TimeLine::signalZoomFactorChanged, this, &TimeLine::slotZoomFactorToWidth);
}
// END OF TimeLine::TimeLine(QWidget * a_pParent)
//==============================================================================

TimeLine::~TimeLine()
{

}

// END OF TimeLine::~TimeLine()
//==============================================================================

QRectF TimeLine::boundingRect() const
{    
    return QRectF(0, 0, m_viewWidth, 40);
}

// END OF QRectF TimeLine::boundingRect() const
//==============================================================================

void TimeLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QBrush redBrush(Qt::red);

    QFont defaultFont = painter->font();
    QFont rulerTextFont("Arial", 10);

    QRectF rec = boundingRect();

    painter->fillRect(rec, redBrush);

    // don't run on initial paint when max frame has not been set yet
    if (m_maxFrame > 0) {

        // only run when timeline width changed
        if (m_widthChanged) {
            setupRuler();
        }

        DisplayMode l_displayMode = Time;
        if((m_displayMode == Frames) || (m_fps == 0.0))
            l_displayMode = Frames;

        // drawing ruler

        int longTickWidth = m_widthPerSegment;
        int mediumTickWidth = int(double(longTickWidth) / double(2));
        int shortTickWidth = (longTickWidth - mediumTickWidth) / 4;

        int accumLongTickWidth = 0;
        int accumMediumTickWidth = 0;
        int accumShortTickWidth = 0;

        for (int i = 0; i < m_maxFrame; i++) {
            // draw long tick and frame/time string and spacing line
            if (i % longTickWidth == 0) {
                painter->drawLine(i, 17, i, 39);
                accumLongTickWidth += longTickWidth;

                // get frame
                int current_frame = posToFrame(i);

                QString labelString;
                if(l_displayMode == Frames)
                    labelString = QVariant(current_frame).toString();
                else {
                    labelString = vsedit::timeToString(double(current_frame) / m_fps);
                }

                QPoint labelPos(i, 15);
                painter->setFont(rulerTextFont);
                painter->drawText(labelPos, labelString);
            }

            // draw medium tick
            if (i % mediumTickWidth == 0 && i % longTickWidth != 0) {
                painter->drawLine(i, 22, i, 39);
                accumMediumTickWidth += mediumTickWidth;
            }

            // draw short tick
            if (i % shortTickWidth == 0 && i % longTickWidth != 0 && i % mediumTickWidth != 0) {
                painter->drawLine(i, 30, i, 39);
                accumShortTickWidth += shortTickWidth;
            }
        }

        m_widthChanged = false;        
    }
}

// END OF void TimeLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
//==============================================================================

void TimeLine::setZoomFactor(int a_zoomFactor)
{
    if (a_zoomFactor >= 1) {
        m_zoomFactor = a_zoomFactor;

        emit signalZoomFactorChanged();
    }
}

// END OF void TimeLine::setZoomFactor(int a_zoomFactor)
//==============================================================================

void TimeLine::reloadZoomFactor() // use when graphicsview was resized
{
    setZoomFactor(m_zoomFactor);
}

// END OF void TimeLine::reloadZoomFactor()
//==============================================================================

void TimeLine::setupRuler()
{
    m_totalSegments = int(double(m_viewWidth) / double(m_widthPerSegment));
    m_framesPerSegment = m_maxFrame / m_totalSegments;
}

// END OF void TimeLine::setupRuler()
//==============================================================================

void TimeLine::setViewWidth(int a_viewWidth)
{    
    if (m_viewWidth == a_viewWidth)
        return;

    m_viewWidth = a_viewWidth;
    prepareGeometryChange();

    m_widthChanged = true;
    emit signalTimeLineWidthChanged();
}

// END OF void TimeLine::setViewWidth(int a_viewWidth)
//==============================================================================

void TimeLine::setBaseWidth(int a_baseWidth)
{
    m_baseWidth = a_baseWidth;
}

// END OF void TimeLine::setBaseWidth(int a_baseWidth)
//==============================================================================

int TimeLine::width()
{
    return m_baseWidth;
}

// END OF int TimeLine::width()
//==============================================================================

int TimeLine::viewWidth() const
{
    return m_viewWidth;
}

// END OF int TimeLine::viewWidth() const
//==============================================================================

/*** original ***/

int TimeLine::frame() const
{
    return m_currentFrame;
}

// END OF int TimeLine::frame() const
//==============================================================================

void TimeLine::setFrameByPos(int a_pos)
{
    int frame = posToFrame(a_pos);
    setFrame(frame);
}
// END OF void TimeLine::setFrameByPos(int a_pos)
//==============================================================================

int TimeLine::currentFramePos()
{
    int pos = frameToPos(m_currentFrame);
    return pos;
}

// END OF int TimeLine::currentFramePos
//==============================================================================

void TimeLine::increaseZoomFactor()
{
    m_zoomIn = true;
    setZoomFactor(m_zoomFactor + 1);
//    qDebug() << "zoom factor: " << m_zoomFactor;
}

// END OF void TimeLine::increaseZoomFactor()
//==============================================================================

void TimeLine::decreaseZoomFactor()
{
    if (m_zoomFactor - 1 >= 1 ) {
        m_zoomOut = true;

        setZoomFactor(m_zoomFactor - 1);
//        qDebug() << "zoom factor: " << m_zoomFactor;
    }
}

// END OF void TimeLine::decreaseZoomFactor()
//==============================================================================

void TimeLine::setStep(int a_step)
{
    if (a_step < 0)
        m_step = 1;

    m_step = a_step;
}

// END OF void TimeLine::setStep(int a_step)
//==============================================================================

void TimeLine::setFrame(int a_frame)
{
    int oldCurrentFrame = m_currentFrame;
    if(a_frame > m_maxFrame)
        m_currentFrame = m_maxFrame;
    else
        m_currentFrame = a_frame;

    if(m_currentFrame == oldCurrentFrame)
        return;

    update();

    emit signalFrameChanged(m_currentFrame);
}

// END OF void TimeLine::setFrame(int a_frame)
//==============================================================================

void TimeLine::setFramesNumber(int a_framesNumber)
{
    m_maxFrame = a_framesNumber - 1;

    if(m_currentFrame > m_maxFrame)
        setFrame(m_maxFrame);
    update();
}

// END OF void TimeLine::setFramesNumber(int a_framesNumber)
//==============================================================================

void TimeLine::setFPS(double a_fps)
{
    m_fps = a_fps;
    update();
}

// END OF void TimeLine::setFPS(double a_fps)
//==============================================================================

TimeLine::DisplayMode TimeLine::displayMode() const
{
    return m_displayMode;
}

// END OF TimeLine::DisplayMode TimeLine::displayMode() const
//==============================================================================

void TimeLine::setDisplayMode(DisplayMode a_displayMode)
{
    m_displayMode = a_displayMode;
    update();
}

// END OF void TimeLine::setDisplayMode(DisplayMode a_displayMode)
//==============================================================================

void TimeLine::addBookmark(int a_bookmark)
{
    if(a_bookmark < 0)
        return;

    m_bookmarks.insert(a_bookmark);
    update();
}

// END OF void TimeLine::addBookmark(int a_bookmark)
//==============================================================================


void TimeLine::removeBookmark(int a_bookmark)
{
    m_bookmarks.erase(a_bookmark);
    update();
}

// END OF void TimeLine::removeBookmark(int a_bookmark)
//==============================================================================

std::set<int> TimeLine::bookmarks() const
{
    return m_bookmarks;
}

// END OF std::set<int> TimeLine::bookmarks() const
//==============================================================================

void TimeLine::setBookmarks(const std::set<int> & a_bookmarks)
{
    std::set<int>::iterator it = a_bookmarks.lower_bound(0);
    m_bookmarks = std::set<int>(it, a_bookmarks.end());
    update();
}

// END OF void TimeLine::setBookmarks(const std::set<int> & a_bookmarks)
//==============================================================================

void TimeLine::clearBookmarks()
{
    m_bookmarks.clear();
    update();
}

// END OF void TimeLine::clearBookmarks()
//==============================================================================

int TimeLine::getClosestBookmark(int a_frame) const
{
    if(m_bookmarks.size() == 0)
        return a_frame;

    if(m_bookmarks.size() == 1)
        return *m_bookmarks.begin();

    std::set<int>::iterator next = m_bookmarks.upper_bound(a_frame);

    if(next == m_bookmarks.begin())
        return *next;

    if(next == m_bookmarks.end())
        return *m_bookmarks.rbegin();

    if(*next > m_maxFrame)
    {
        next = m_bookmarks.upper_bound(m_maxFrame);
        next--;
        return *next;
    }

    std::set<int>::iterator prev = next;
    prev--;

    int backDiff = a_frame - *prev;
    int forwardDiff = *next - a_frame;

    if(forwardDiff < backDiff)
        return *next;

    return *prev;
}

// END OF int TimeLine::getClosestBookmark(int a_frame) const
//==============================================================================


int TimeLine::posToFrame(int a_pos) const
{
    int frameNum = int(double(a_pos) / double(m_viewWidth) * double(m_maxFrame));
    return frameNum;
}

// END OF int TimeLine::posToFrame(int a_pos) const
//==============================================================================

int TimeLine::frameToPos(int a_frame) const
{
    if(a_frame > m_maxFrame)
        a_frame = m_maxFrame;

    int framePos = int(double(a_frame) * double(m_viewWidth) / double(m_maxFrame));

    return framePos;
}

// END OF int TimeLine::frameToPos(int a_frame) const
//==============================================================================

void TimeLine::slotStepUp()
{
    if(m_currentFrame < m_maxFrame) {
        setFrame(m_currentFrame + m_step);
    }
}

// END OF void TimeLine::slotStepUp()
//==============================================================================

void TimeLine::slotStepDown()
{
    if(m_currentFrame > 0) {
        if (m_currentFrame - m_step < 0) {
            setFrame(0);
        } else {
            setFrame(m_currentFrame - m_step);
        }
    }
}

// END OF void TimeLine::slotStepDown
//==============================================================================

void TimeLine::slotZoomFactorToWidth()
{
    double scaleMultiplier = 0.0;
    int updated_width;

    if (m_zoomIn) {
        scaleMultiplier = double(1) + 0.2 * double(m_zoomFactor - 1);
        m_accuScaleMultiplier += scaleMultiplier;
        m_zoomIn = false;
    }

    if (m_zoomOut) {
        scaleMultiplier = double(1) + 0.2 * double(m_zoomFactor);
        m_accuScaleMultiplier -= scaleMultiplier;
        m_zoomOut = false;
    }

//    qDebug() << "scale multiplier : " << scaleMultiplier;
//    qDebug() << "accumulative scale multiplier : " << m_accuScaleMultiplier;

    if (m_zoomFactor == 1) {
        updated_width = m_baseWidth;
    } else {
        updated_width = int(double(m_baseWidth) * double(m_accuScaleMultiplier));
    }

    // if width exceeded limit 32,768, scale back zoom factor and m_accuScaleMultiplier, and exit
    if (updated_width > pow(2, 15)) {
        m_zoomFactor -= 1;
        m_accuScaleMultiplier -= scaleMultiplier;
        return;
    }

    setViewWidth(updated_width);
}

// END OF void TimeLine::slotZoomFactorToWidth()
//==============================================================================

void TimeLine::slotGoToPreviousBookmark()
{
    if(m_bookmarks.size() == 0)
        return;

    if(m_currentFrame == 0)
        return;

    std::set<int>::iterator it = m_bookmarks.upper_bound(m_currentFrame - 1);

    if(it == m_bookmarks.end())
    {
        setFrame(*m_bookmarks.rbegin());
        return;
    }

    if(it == m_bookmarks.begin())
        return;

    it--;

    if(*it < 0)
        return;

    setFrame(*it);

}

void TimeLine::slotGoToNextBookmark()
{
    if(m_bookmarks.size() == 0)
        return;

    std::set<int>::iterator it = m_bookmarks.upper_bound(m_currentFrame);

    if(it == m_bookmarks.end())
        return;

    if(*it > m_maxFrame)
        return;

    setFrame(*it);

}
