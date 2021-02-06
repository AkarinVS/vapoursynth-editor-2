#include "../../src/selection_tools/line_painter.h"

#include "canvas.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QHBoxLayout>

Canvas::Canvas(QWidget *a_pParent): QScrollArea(a_pParent),
    m_selectionTool(SelectionTools::Rectangle),
    m_zoomRatio(1.0)
{
    m_pLinePainter = new LinePainter(this);
    setAlignment(Qt::AlignCenter);
    setWidget(m_pLinePainter);
    setMouseTracking(true);
}

void Canvas::wheelEvent(QWheelEvent *a_pEvent)
{
    QPoint delta = a_pEvent->angleDelta();
    // zooming
    if (delta.y() > 0) {
        m_zoomRatio += 0.3;
    } else {
        if (m_zoomRatio - 0.3 <= 0.0 ) return; // cap min at zero
        m_zoomRatio -= 0.3;
    }

    QSize zoomSize = m_framePixmap.size() * m_zoomRatio;
    m_pLinePainter->setSelectionPixmap(m_framePixmap.scaled(zoomSize));
    m_pLinePainter->setFixedSize(m_framePixmap.size() * m_zoomRatio);
}

void Canvas::slotSetSelectionTool(SelectionTools a_selectedTool)
{
    if (m_framePixmap.isNull()) return;
    m_pLinePainter->setSelectionTool(a_selectedTool);
}

void Canvas::drawFrame(const QPixmap &a_framePixmap)
{
    m_framePixmap = a_framePixmap.copy(); // save a copy for zooming

    m_pLinePainter->setFixedSize(a_framePixmap.size());
    m_pLinePainter->setSelectionPixmap(a_framePixmap);
}

QString Canvas::selectionXYToString()
{
    /* retrieve selection xy from linepainter and revert them back to original ratio */
    double revertFactor = double(m_framePixmap.width())
                        / double(m_framePixmap.width() * m_zoomRatio);

    auto rectXY = m_pLinePainter->selectionRectXY();

    if (rectXY == QVector({0.0,0.0,0.0,0.0})) return QString("");

    if (revertFactor == 1.0) {
        return QString("%1, %2, %3, %4").arg(int(rectXY[0])).arg(int(rectXY[1]))
                .arg(int(rectXY[2])).arg(int(rectXY[3]));
    } else {
        int convertedMinX = int(rectXY[0]) * int(revertFactor);
        int convertedMinY = int(rectXY[1]) * int(revertFactor);
        int convertedMaxX = int(rectXY[2]) * int(revertFactor);
        int convertedMaxY = int(rectXY[3]) * int(revertFactor);

        return QString("%1, %2, %3, %4").arg(convertedMinX).arg(convertedMinY)
                .arg(convertedMaxX).arg(convertedMaxY);
    }
}
