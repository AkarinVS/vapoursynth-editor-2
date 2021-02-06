#ifndef FRAMEPAINTER_H
#define FRAMEPAINTER_H

#include <QObject>
#include <QWidget>

class FramePainter : public QWidget
{
    Q_OBJECT
public:
    explicit FramePainter(QWidget *parent = nullptr);

    void virtual paintEvent(QPaintEvent *) override;

    void drawFrame(const QPixmap &a_framePixmap);

    void setRatio(const double a_ratio);

    QPixmap *pixmap();

private:

    QPixmap m_framePixmap;

    double m_ratio;

signals:

};

#endif // FRAMEPAINTER_H
