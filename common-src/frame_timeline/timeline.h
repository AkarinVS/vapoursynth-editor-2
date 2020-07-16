#ifndef TIMELINE_H
#define TIMELINE_H

#include <QGraphicsObject>
#include <QWidget>
#include <QPainter>
#include <set>

using namespace std;

class TimeLine : public QGraphicsObject
{
    Q_OBJECT

public:
    TimeLine(QWidget * a_pParent = nullptr);

    virtual ~TimeLine() override;

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    enum DisplayMode
    {
        Time,
        Frames,
    };

    int zoomFactor();
    void setZoomFactor(int a_zoomFactor);

    void reloadZoomFactor();

    void setViewWidth(int a_viewWidth);

    void setBaseWidth(int a_baseWidth);

    int width();

    int viewWidth() const;

    void setFrameByPos(int a_pos);

    int frameToPos(int a_frame) const;

    int posToFrame(int a_pos) const;

    int currentFramePos();

    void increaseZoomFactor();

    void decreaseZoomFactor();

    void setStep(int a_step);


    /**** original ****/
    int frame() const;

    void setFrame(int a_frame);

    void setFramesNumber(int a_framesNumber);

    void setFPS(double a_fps);

    double fps();

    DisplayMode displayMode() const;

    void setDisplayMode(DisplayMode a_displayMode);

    // bookmarks
    void addBookmark(int a_bookmark);
    void removeBookmark(int a_bookmark);
    std::set<int> bookmarks() const;
    void setBookmarks(const std::set<int> & a_bookmarks);
    void clearBookmarks();
    int getClosestBookmark(int a_frame) const;

private:

    int m_baseWidth;
    int m_viewWidth;

    // navgation
    int m_step;

    // ruler
    int m_totalSegments;
    int m_framesPerSegment;
    int m_widthPerSegment;

    // zooming
    int m_zoomFactor;
    int m_zoomedWidth;
    double m_accuScaleMultiplier;
    bool m_zoomIn, m_zoomOut;
    bool m_maxZoomIn = false;

    int m_maxFrame;
    double m_fps;

    int m_currentFrame;

    DisplayMode m_displayMode;

    // bookmarks
    std::set<int> m_bookmarks;


public slots:

    void slotStepUp();
    void slotStepDown();

    void slotZoomFactorToWidth();

    // bookmarks
    void slotGoToPreviousBookmark();
    void slotGoToNextBookmark();


signals:

    void signalFrameChanged(int a_frame);

    void signalZoomFactorChanged();

    void signalTimeLineWidthChanged();

};

#endif // TimeLine_H
