#ifndef PREVIEWAREA_H
#define PREVIEWAREA_H

#include <QScrollArea>
#include <QPixmap>
#include <QPoint>

class QLabel;
class ScrollNavigator;
class QKeyEvent;
class QWheelEvent;
class QMouseEvent;
class FramePainter;

class PreviewArea : public QScrollArea
{
	Q_OBJECT

public:

    PreviewArea(QWidget * a_pParent = nullptr);

    virtual ~PreviewArea() override;

    QPixmap *framePixmap() ;

    FramePainter *m_pFramePainter;

    void setPixmap(const QPixmap & a_pixmap, const double a_ratio);

    void setZoomRatio(const double a_ratio);

    void setPreviewScrollBarPos(const QPair<int,int>&);

    void checkMouseOverPreview(const QPoint & a_globalMousePos);      

public slots:

	void slotScrollLeft();
	void slotScrollRight();
	void slotScrollTop();
	void slotScrollBottom();

protected:

	void resizeEvent(QResizeEvent * a_pEvent) override;
	void keyPressEvent(QKeyEvent * a_pEvent) override;
	void wheelEvent(QWheelEvent * a_pEvent) override;
	void mousePressEvent(QMouseEvent * a_pEvent) override;
	void mouseMoveEvent(QMouseEvent * a_pEvent) override;
	void mouseReleaseEvent(QMouseEvent * a_pEvent) override;

signals:

    void signalKeyPressed(Qt::Key a_enumKey);
	void signalSizeChanged();
	void signalCtrlWheel(QPoint a_angleDelta);
	void signalMouseMiddleButtonReleased();
	void signalMouseRightButtonReleased();
    void signalMouseOverPoint(float a_normX, float a_normY);
    void signalPreviewScrollBarsPosChanged(const QPair<int, int>&);

private:

	void drawScrollNavigator();

	ScrollNavigator * m_pScrollNavigator;

	bool m_draggingPreview;
	QPoint m_lastCursorPos;
    QPoint m_lastFramePainterPos;
};

#endif // PREVIEWAREA_H
