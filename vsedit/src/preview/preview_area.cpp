#include "preview_area.h"
#include "frame_painter.h"

#include "scroll_navigator.h"

#include <QLabel>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QCoreApplication>
#include <QDebug>

//==============================================================================

PreviewArea::PreviewArea(QWidget * a_pParent) : QScrollArea(a_pParent)
	, m_pScrollNavigator(nullptr)
	, m_draggingPreview(false)
	, m_lastCursorPos(0, 0)
    , m_lastFramePainterPos(0, 0)
{
    m_pFramePainter = new FramePainter(this);
    setWidget(m_pFramePainter);

    setMouseTracking(true);
    m_pFramePainter->setMouseTracking(true);

    m_pScrollNavigator = new ScrollNavigator(this);
    int scrollFrameWidth = frameWidth();
    m_pScrollNavigator->move(pos() +
        QPoint(scrollFrameWidth, scrollFrameWidth));
    m_pScrollNavigator->setVisible(false);
}

// END OF PreviewArea::PreviewArea(QWidget * a_pParent)
//==============================================================================

PreviewArea::~PreviewArea()
{

}

// END OF PreviewArea::~PreviewArea()
//==============================================================================

QPixmap PreviewArea::pixmap()
{
    return m_pFramePainter->pixmap();
}

// END OF const QPixmap * PreviewArea::pixmap() const
//==============================================================================

void PreviewArea::setPixmap(const QPixmap & a_pixmap, const qreal a_ratio)
{
    m_pFramePainter->resize(int(double(a_pixmap.width()) * a_ratio),
                            int(double(a_pixmap.height()) * a_ratio));
    m_pFramePainter->setRatio(a_ratio);
    m_pFramePainter->drawFrame(a_pixmap);
}

void PreviewArea::setPreviewScrollBarPos(const QPair<int, int> &a_posPair)
{
    horizontalScrollBar()->setValue(a_posPair.first);
    verticalScrollBar()->setValue(a_posPair.second);
}

// END OF void PreviewArea::setPixmap(const QPixmap & a_pixmap)
//==============================================================================


void PreviewArea::checkMouseOverPreview(const QPoint & a_globalMousePos)
{
    if(!m_pFramePainter->underMouse())
        return;

    QPoint imagePoint = m_pFramePainter->mapFromGlobal(a_globalMousePos);

    QSize pixmapSize = m_pFramePainter->size();
    int pixmapWidth = pixmapSize.width();
    int pixmapHeight = pixmapSize.height();

    if((imagePoint.x() < 0) || (imagePoint.y() < 0) ||
        (imagePoint.x() >= pixmapWidth) || (imagePoint.y() >= pixmapHeight))
        return;

    float normX = float(imagePoint.x()) / float(pixmapWidth);
    float normY = float(imagePoint.y()) / float(pixmapHeight);

    emit signalMouseOverPoint(normX, normY);
}

// END OF void PreviewArea::checkMouseOverPreview(
//		const QPoint & a_globalMousePos)
//==============================================================================

void PreviewArea::slotScrollLeft()
{
	horizontalScrollBar()->setValue(0);
}

// END OF void PreviewArea::slotScrollLeft()
//==============================================================================

void PreviewArea::slotScrollRight()
{
	QCoreApplication::processEvents();
	QScrollBar * pHorizontalScrollbar = horizontalScrollBar();
	pHorizontalScrollbar->setValue(pHorizontalScrollbar->maximum());
}

// END OF void PreviewArea::slotScrollRight()
//==============================================================================

void PreviewArea::slotScrollTop()
{
	verticalScrollBar()->setValue(0);
}

// END OF void PreviewArea::slotScrollTop()
//==============================================================================

void PreviewArea::slotScrollBottom()
{
	QCoreApplication::processEvents();
	QScrollBar * pVerticalScrollbar = verticalScrollBar();
	pVerticalScrollbar->setValue(pVerticalScrollbar->maximum());
}

// END OF void PreviewArea::slotScrollBottom()
//==============================================================================

void PreviewArea::resizeEvent(QResizeEvent * a_pEvent)
{
	QScrollArea::resizeEvent(a_pEvent);
    update();
	emit signalSizeChanged();
}

// END OF void PreviewArea::resizeEvent(QResizeEvent * a_pEvent)
//==============================================================================

void PreviewArea::keyPressEvent(QKeyEvent * a_pEvent)
{
	if(a_pEvent->modifiers() != Qt::NoModifier)
	{
		QScrollArea::keyPressEvent(a_pEvent);
		return;
	}

    int key = a_pEvent->key();

    if (key == Qt::Key_Left)
        emit signalKeyPressed(Qt::Key_Left);
    else if (key == Qt::Key_Right)
        emit signalKeyPressed(Qt::Key_Right);
    else if (key == Qt::Key_Home)
        emit signalKeyPressed(Qt::Key_Home);
    else if (key == Qt::Key_End)
        emit signalKeyPressed(Qt::Key_End);
    else
        QScrollArea::keyPressEvent(a_pEvent);
}

// END OF void PreviewArea::keyPressEvent(QKeyEvent * a_pEvent)
//==============================================================================

void PreviewArea::wheelEvent(QWheelEvent * a_pEvent)
{
	if(a_pEvent->modifiers() == Qt::ControlModifier)
	{
		emit signalCtrlWheel(a_pEvent->angleDelta());
		a_pEvent->ignore();
		return;
	}
    a_pEvent->ignore();
//	QScrollArea::wheelEvent(a_pEvent);
}

// END OF void PreviewArea::wheelEvent(QWheelEvent * a_pEvent)
//==============================================================================

void PreviewArea::mousePressEvent(QMouseEvent * a_pEvent)
{
    if(a_pEvent->buttons() == Qt::LeftButton)
    {
        m_draggingPreview = true;
        m_lastCursorPos = a_pEvent->globalPos();
        m_lastFramePainterPos = m_pFramePainter->pos();
        m_pScrollNavigator->setVisible(true);
        drawScrollNavigator();
        a_pEvent->accept();
        return;
    }

	QScrollArea::mousePressEvent(a_pEvent);
}

// END OF void PreviewArea::mousePressEvent(QMouseEvent * a_pEvent)
//==============================================================================

void PreviewArea::mouseMoveEvent(QMouseEvent * a_pEvent)
{
    if((a_pEvent->buttons() & Qt::LeftButton) && m_draggingPreview)
    {
        QPoint newCursorPos = a_pEvent->globalPos();
        QPoint posDifference = newCursorPos - m_lastCursorPos;
        QPoint newFramePainterPos = m_lastFramePainterPos +
            posDifference;

        int scrollBarHorizontalPos = -newFramePainterPos.x();
        int scrollBarVerticalPos = -newFramePainterPos.y();

        emit signalPreviewScrollBarsPosChanged(QPair(scrollBarHorizontalPos, scrollBarVerticalPos));
        horizontalScrollBar()->setValue(scrollBarHorizontalPos);
        verticalScrollBar()->setValue(scrollBarVerticalPos);

        drawScrollNavigator();
        a_pEvent->accept();
        return;
    }
    QPoint globalPoint = a_pEvent->globalPos();
    checkMouseOverPreview(globalPoint);
    QScrollArea::mouseMoveEvent(a_pEvent);
}

// END OF void PreviewArea::mouseMoveEvent(QMouseEvent * a_pEvent)
//==============================================================================

void PreviewArea::mouseReleaseEvent(QMouseEvent * a_pEvent)
{
    Qt::MouseButton releasedButton = a_pEvent->button();
    if(releasedButton == Qt::LeftButton)
    {
        m_draggingPreview = false;
        m_pScrollNavigator->setVisible(false);
        a_pEvent->accept();
        return;
    }
    else if(releasedButton == Qt::MidButton)
        emit signalMouseMiddleButtonReleased();
    else if(releasedButton == Qt::RightButton)
        emit signalMouseRightButtonReleased();

    QScrollArea::mouseReleaseEvent(a_pEvent);
}

// END OF void PreviewArea::mouseReleaseEvent(QMouseEvent * a_pEvent)
//==============================================================================

void PreviewArea::drawScrollNavigator()
{
    int contentsWidth = m_pFramePainter->pixmap().width();
    int contentsHeight = m_pFramePainter->pixmap().height();
    int viewportX = -m_pFramePainter->x();
    int viewportY = -m_pFramePainter->y();
	int viewportWidth = viewport()->width();
	int viewportHeight = viewport()->height();

	m_pScrollNavigator->draw(contentsWidth, contentsHeight, viewportX,
		viewportY, viewportWidth, viewportHeight);
}

// END OF void PreviewArea::drawScrollNavigator()
//==============================================================================
