#ifndef CANVAS_H
#define CANVAS_H

#include "selection_tools_dialog.h"
#include <QObject>
#include <QWidget>
#include <QScrollArea>

class LinePainter;

//enum SelectionTools {
//    Rectangle,
//    LazyRectangle
//};

class Canvas : public QScrollArea
{
    Q_OBJECT

public:
    Canvas(QWidget *a_pParent = nullptr);

    void drawFrame(const QPixmap&);
    QString selectionXYToString();

protected:

    void wheelEvent(QWheelEvent * a_pEvent) override;

private:
    void drawPointTo(const QPointF & a_endPoint);
    void resizeImage(QImage *image, const QSize &newSize);

    QPixmap m_framePixmap;
    SelectionTools m_selectionTool;
    LinePainter *m_pLinePainter;

    double m_zoomRatio;

public slots:

    void slotSetSelectionTool(SelectionTools);

};

#endif // CANVAS_H
