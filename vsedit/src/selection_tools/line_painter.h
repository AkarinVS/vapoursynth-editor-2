#ifndef LINEPAINTER_H
#define LINEPAINTER_H

#include "selection_tools_dialog.h"
#include <QObject>
#include <QWidget>

//enum SelectionTools {
//    Rectangle,
//    LazyRectangle
//};

class LinePainter : public QWidget
{
    Q_OBJECT

public:
    LinePainter(QWidget * parent = nullptr);

    void setSelectionPixmap(const QPixmap &);
    void updateSelectionRect();
    void setSelectionTool(SelectionTools);
    QVector<double> selectionRectXY();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
//    void resizeEvent(QResizeEvent *event) override;

    void paintEvent(QPaintEvent * event) override;

private:
    void drawPointTo(const QPointF & a_endPoint);
    void resizeImage(QImage *image, const QSize &newSize);
    void createSelectedRect();

//    SelectionTools m_selectionTool;
//    QPixmap m_framePixmap;
    QPixmap m_selectionPixmap;
    QPixmap m_initialPixmapCopy;

    SelectionTools m_selectionTool;
    QPointF m_lastPoint;
    bool m_drawing;
    double m_scaleFactor;

    double m_minX, m_maxX, m_minY, m_maxY;
    QVector<QPointF> m_pathPoints;

};

#endif // LINEPAINTER_H
